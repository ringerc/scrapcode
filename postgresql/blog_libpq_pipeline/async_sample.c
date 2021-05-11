#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <poll.h>
#include <libpq-fe.h>

/*
 * This is a program for an async event loop based libpq.
 *
 * It'll get pipeline support added soon, via a deeper queue of work items.
 */

/*
 * Support gcc and clang's compile-time checking for __attribute__ support.
 */
#ifdef __has_attribute
#if __has_attribute(format)
#define ATTR_FORMAT(x,y,z) __attribute__((format(x,y,z)))
#endif
#endif
#ifndef ATTR_FORMAT
#define ATTR_FORMAT(x,y,z)
#endif

#ifdef __has_attribute
#if __has_attribute(noreturn)
#define ATTR_NORETURN __attribute__((noreturn))
#endif
#endif
#ifndef ATTR_NORETURN
#define ATTR_NORETURN
#endif

/* States for sessions */
enum SessionState {
    SESSION_IDLE,     /* Nothing on conn, need to send query */
    SESSION_QUEUED,   /* Query in local send buf, pending flush */
    SESSION_FLUSHED   /* waiting for results from server */
};

struct AppSession;
struct QueryQueueItem;

/*
 * Callback to dispatch a single query from a QueryQueueItem onto the session
 * respresented by AppSession.
 *
 * Not required to PGflush(), the async runner will take care of that.
 *
 * May read and/or write the queue item's private storage.
 *
 * Returns the result of the underlying PQsendQuery or PQsendQueryParams() used
 * to dispatch the query.
 */
typedef int (*QueryQueueSendCallback)(struct AppSession * const session, struct QueryQueueItem * const queueitem);

/*
 * Callback to process the results from a matching query send callback for a
 * QueryQueueItem item.
 *
 * May read and/or write the queue item's private storage. Must not free the
 * queue item.
 */
typedef void (*QueryQueueResultCallback)(struct AppSession * const session, struct QueryQueueItem * const queueitem, PGresult *result);

/*
 * Free all memory associated with a QueryQueueItem's private storage,
 * and free the queue item itself if needed.
 *
 * It is not guaranteed that the send() or result() callbacks have been invoked
 * when this is called.
 *
 * This callback will be called exactly once for each queue item.
 */
typedef void (*QueryQueueFreeCallback)(struct AppSession * const session, struct QueryQueueItem * const queueitem);

/*
 * An enqueued query, represented as "send" and "result" callbacks
 * plus a private storage area and a callback to free it before
 * the queue item itself is freed.
 *
 * The queue is a simply linked list.
 */
#define QUERY_QUEUE_ITEM_COMMENT_MAXLEN 63
struct QueryQueueItem {
    struct QueryQueueItem *     next;
    QueryQueueSendCallback      send_cb;
    QueryQueueResultCallback    result_cb;
    QueryQueueFreeCallback      free_cb;
    /*
     * Private storage for callbacks to use. Usually a pointer but doesn't have
     * to be. Any resources it points to must be freed by a call to free_cb.
     */
    uintptr_t                   cb_private;
    /*
     * Short text comment describing the item. This could be a variable array
     * member or a pointer allocated out of line, but this is easier and
     * doesn't require another allocation. Downside is that the queue items
     * won't fit in catche as nicely.
     */
    char                        comment[QUERY_QUEUE_ITEM_COMMENT_MAXLEN];
};

/*
 * Handle any asynchronous notifications received on the connection, if any.
 *
 * Optional.
 *
 * Must NOT call PQfreemem() on the notification. The notification will be
 * freed after this call returns.
 */
typedef void (*AppSessionNotifyCallback)(struct AppSession * const session, const PGnotify * const notification);

/*
 * State management for the connection to track the connection, what stage a
 * query on the session is at, and any query queue.
 */
struct AppSession {
    PGconn                *conn;
    enum SessionState        session_state;
    struct QueryQueueItem *queue_head;
    struct QueryQueueItem *queue_tail;
    /* Handle asynchronous notifications */
    AppSessionNotifyCallback notify_cb;
};

/*
 * Sessions are managed in a single global array for this toy example, for ease
 * of error cleanup on bail-out. nsessions_slots is the available storage in
 * the array, nsessions is the number used.  Sessions are filled left-to-right
 * and no holes are permitted.
 */
static struct AppSession * sessions = NULL;
static int nsessions_slots = 0;
static int nsessions = 0;

/*
 * Populate and enqueue a QueryQueueItem, setting the appropriate callbacks
 * and private data area.
 *
 * The QueryQueueItem's storage must be provided by the caller. If it's
 * malloc'd storage, the caller must provide a free callback. If it's on the
 * stack and doesn't point to anything else the free callback may be null.
 */
static void push_query_queue_item(
        struct AppSession * const session,
        struct QueryQueueItem * const queueitem,
        QueryQueueSendCallback send_cb,
        QueryQueueResultCallback result_cb,
        QueryQueueFreeCallback free_cb,
        uintptr_t cb_private,
        const char * comment
        )
{
    assert(send_cb != NULL && result_cb != NULL);
    assert(queueitem != NULL);

    queueitem->send_cb = send_cb;
    queueitem->result_cb = result_cb;
    queueitem->free_cb = free_cb;
    queueitem->cb_private = cb_private;
    if (comment) {
        strncpy(queueitem->comment, comment, QUERY_QUEUE_ITEM_COMMENT_MAXLEN);
        queueitem->comment[QUERY_QUEUE_ITEM_COMMENT_MAXLEN] = '\0';
    }
    /* Actual enqueue */
    if (session->queue_head == 0)
        assert(session->queue_tail == 0);
        session->queue_head = queueitem;
    } else {
        assert(session->queue_tail != 0);
        assert(session->queue_tail->next == 0);
        session->queue_tail->next = queueitem;
    }
    session->queue_tail = queueitem;
}

/*
 * Discard the top queue item from the stack and call its
 * free callback.
 *
 * The free callback is invoked only once the item is already popped from the
 * queue.
 */
static void pop_query_queue_item(struct AppSession * const session)
{
    struct QueryQueueItem * old_head = session->queue_head;
    assert(old_head != NULL);
    session->queue_head = old_head->next;
    if (session->queue_head == 0)
        session->queue_tail = 0;
    if (old_head->free_cb)
        old_head->free_cb(session, old_head);
}

/*
 * Kill all active sessions on error recovery path.
 *
 * If we don't do this then the server will get an error about the socket
 * unexpectedly closing instead of a more sensible disconnect.
 */
static void kill_sessions(void)
{
    while (nsessions > 0)
    {
        struct AppSession * const session = &sessions[nsessions - 1];
        struct QueryQueueItem * queueitem = session->queue_head;
        while (session->queue_head)
            pop_query_queue_item(session);
        if (session->conn) {
            PQfinish(session->conn);
            session->conn = 0;
        }
        nsessions --;
    }
}

/*
 * Simple error bailout function to close any open conns and print an error
 * message.
 *
 * Yours will differ based on your application's needs; you might be using C++
 * exceptions, longjmp() based error handling, long return chains, or whatever
 * else. This is deliberately a simple and direct bail-out to just to minimise
 * the impact of error handling on the example code.
 */
ATTR_FORMAT(__printf__,1,2) ATTR_NORETURN
static void error_die(const char * const fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (conn) {
        PQfinish(conn);
    }

    vfprintf(stderr, fmt, args);

    kill_sessions();

    free(sessions);
    sessions = 0;
    nsessions_storage = 0;

    exit(1);
}

/*
 * Establish a connection to the database.
 *
 * It's better to use use async connect here instead of normal synchronous
 * connect, but there's some hoop jumping involved with socket file descriptors
 * and wait states, so we'll keep it simple. That way the example can focus on
 * async and pipelined queries.
 */
static void dbconnect(struct AppSession * session, const char *connstr) {

    /* TODO async */
    session->conn = PQconnectdb(connstr);

    if (PQstatus(session->conn) != CONNECTION_OK)
        error_die("connection failed: %s", PQerrorMessage(session->conn));

    /* If the send buffer is full, don't block. Has no effect on receive. */
    if (PQsetnonblocking(conn, 1) != 0)
        error_die("couldn't set nonblocking mode: %s", PQerrorMessage(session->conn));

    session->session_state = SESSION_IDLE;
    session->queue_head = 0;
    session->queue_tail = 0;
}

/*
 * Run a stream of generated queries forever, without blocking.
 */
static void sample_mainloop(void);
    int              s;
    struct pollfd   *sesspolls;

    /* Storage for poll() state */
    sesspoll = malloc(nsessions * sizeof(struct pollfd))

    for (;;) {

        /* Do any query sending, results processing etc on each session */
        for (s = 0; s < nsessions; s++) {
            struct AppSession * const session = &sessions[s];
            service_session(session, sesspoll[s].revents & POLLIN);
        }

        /* Prepare to poll() for readable/writeable sockets */
        for (s = 0; s < nsessions; s++) {
            /*
             * We don't really have to get the socket fd
             * each time, but it's cheap and this way we'd cope correctly if
             * something disconnected/reconnected the underlying PGconn for a
             * session out from under us.
             */
            sesspoll[s].fd = PQsocket(session->conn);

            /*
             * We always want to read from any readable socket. But don't wake
             * for writeable sockets unless a query is ready to send and the
             * connection is ready for one.
             */
            if (session->session_state == SESSION_QUEUED
                    || (session->session_state == SESSION_IDLE && session->queue_head))
                sesspoll[s].events = POLLIN|POLLOUT;
            else
                sesspoll[s].events = POLLIN;
        }

        /*
         * Sleep until there's something to read or write on a socket (or we're
         * interrupted by a signal).
         *
         * In a real-world program you'd be doing this in your main event loop
         * along with handling various other events, using poll(), select(),
         * WaitForMultipleObjectsEx(), WaitEventSetWait(), etc.
         */
        if (poll(connpoll, 1, -1) < 0)
            error_die("error poll()ing for socket events: %s", strerror(errno));

    }
}

/*
 * Service a given connection in a non-blocking manner. Sends a new query if
 * the session is idle, drains any pending results, calls result handling
 * callbacks if results are ready, etc.
 *
 * Sets POLLOUT in the connpoll state if we're currently waiting for this
 * socket to become writeable in order to be able to progress.
 */
static void service_session(struct * const AppSession session, int readable) {

    /*
     * If socket is readable, drain socket buffer to libpq's internal buffer.
     * Doesn't handle any results yet. Don't assume that reads are non-blocking
     * if nothing is readable.
     *
     * If libpq receive buffer size is a concern, apps can use row-at-a-time
     * mode.
     */
    if (readable) {
        if (PQconsumeInput(session->conn) != 1)
            error_die("could not read from socket: %s", PQerrorMessage(session->conn));
    }

    /*
     * Dispatch asynchronous notifications. Even if there isn't any handler for
     * them registered, we'll pop and free them to make sure they don't pile
     * up.
     */
    {
        PGnotify * const notification = PQnotifies(session->conn);
        if (notification != NULL) {
            if (session->notify_cb)
                session->notify_cb(session, notification);
            PQfreemem(notification);
        }
    }

    /*
     * Handle the actual query sending and results processing.
     */
    switch (session->session_state) {

        /*
         * If the connection is idle and there's a query to send pending,
         * put the query on the internal libpq send buffer.
         */
        case SESSION_IDLE:
        {
            QueryQueueItem * const queueitem = session->queue_head;
            if (queueitem) {
                int send_res = session->send_cb(session, queueitem);
                if (send_res != 1)
                    error_die("could not add query to libpq send buffer: %s", PQerrorMessage(session->conn));
                session->session_state = QUERY_SENT;
            }
            /* FALLTHRU */
        }

        /*
         * There's a query on the internal libpq send-buffer waiting to go to
         * the OS's socket send buffer. If there's enough room on the OS
         * socket send-buffer, flush it and start waiting for results.
         */
        case QUERY_SENT:
        {
            int flushret = PQflush(session->conn);
            if (flushret == -1) {
                error_die("socket error while flushing libpq send buffer: %s", PQerrorMessage(session->conn));
            }
            else if (flushret == 1) {
                /* It's in flight as far as we're concerned, wait for results now */
                session->session_state = SESSION_FLUSHED;
            }
            else {
                /*
                 * Socket sndbuf full, must wait for writable socket to finish
                 * flushing pending query to server. Still wake for inbound
                 * data as that'll help prevent a client/server buffer-full
                 * deadlock, and let us recieve async notifications.
                 */
                assert(flushret == 0);
                sesspoll->events |= POLLOUT;
            }
            break;
        }

        /*
         * We're waiting to finish receiving the server's response(s) to
         * a query.
         */
        case SESSION_FLUSHED:
        {
            /*
             * Consume multiple response items at once if they're available, so
             * that we don't have to do a poll() syscall for each item. This mostly
             * matters if the libpq connection is currently in row-at-a-time mode,
             * but it also lets us consume the NULL PGresult required by the API
             * without another loop.
             *
             * A real app would want to limit this loop so that one busy session
             * doesn't starve another (assuming no threading).
             */
            while (!PQisBusy(conn)) {
                /* Got a reply from the server */
                PGresult * res = PQgetResult(conn);

                /*
                 * Let the result handler for the queue item know. The callback
                 * must PQclear() the result.
                 *
                 * If the result is not NULL we must loop over PQgetResult
                 * until we do get a NULL result, per the libpq API contract.
                 *
                 * NULL reply means no more results, query is done and
                 * connection is idle. Even if the result is a NULL marker for
                 * end-of-results, the handler might want to know so it can do
                 * any final actions.
                 */
                session->result_cb(session, session->queue_head, res);
                if (res == NULL)
                {
                    pop_query_queue_item(session);
                    /*
                     * We should be able to send a new query immediately, but
                     * to simplify flow control we'll wait for the next poll()
                     * cycle where the socket is writeable.
                     */
                    session_state = SESSION_IDLE;
                    sesspoll->events |= POLLOUT;
                }
            }
            break;
        }
    }

    /* Control returns to the mainloop */
}

/* QueryQueueSendCallback for simple_command_enqueue */
static void send_simple_command(struct AppSession * const session, struct QueryQueueItem * const queueitem) {
   return PQsendQuery(session->conn, (const char *)queueitem->cb_private);
}

/* QueryQueueResultCallback for simple_command_enqueue */
static void simple_command_result(struct AppSession * const session, struct QueryQueueItem * const queueitem, PGresult * const result) {
    if (result) {
        if (PQresultStatus(result) != PGRES_COMMAND_OK) {
            error_die("simple command \"%s\" failed with %s: %s\n",
                    (const char *)queueitem->cb_private, PQresStatus(result),
                    PQresultErrorMessage(result));
        }
        PQclear(result);
    }
}

/*
 * QueryQueueFreeCallback for simple_command_enqueue.
 *
 * Since the queue item was malloc'd we must free it explicitly.
 */
static void simple_command_free(struct AppSession * const session, struct QueryQueueItem * const queueitem) {
    free((char*)queueitem->cb_private);
    free(queueitem);
}

/*
 * Put a simple command query on the queue. It must take no parameters and
 * return PGRES_COMMAND_OK. The querytext is stored directly in the private
 * pointer.
 *
 * Query text is copied into a private buffer.
 */
static void simple_command_enqueue(struct AppSession * const session, const char *querytext, const char * const comment) {
    struct QueryQueueItem * queueitem = malloc(sizeof(struct QueryQueueItem));
    push_query_queue_item(session, queueitem,
            simple_command_send, simple_command_result, simple_command_free,
            (uintptr_t)strdup(querytext), comment);
}

/* QueryQueueSendCallback for insert_query */
static void insert_query_send(struct AppSession * const session, struct QueryQueueItem * const queueitem) {
    static const char insert_query[] = {"INSERT INTO demo(id, dummyval) VALUES (DEFAULT, %d) RETURNING (id, dummyval)"};
    char dummyval[20];
    char * vals[1];
    static int query_counter = 0;

    if (snprintf(dummyval, sizeof(dummyval), "%d", (int)queueitem->cb_private) >= sizeof(dummyval))
        error_die("buffer size for parameter insufficient");

    vals[0] = dummyval;

    return PQsendQueryParams(conn, query, 1, NULL, vals, NULL, NULL, 0);
}

/* QueryQueueResultCallback for insert_query */
static void insert_query_result(struct AppSession * const session, struct QueryQueueItem * const queueitem, PGresult * const result) {
    if (result != NULL) {
        if (PQresultStatus(result) == PGRES_TUPLES_OK) {
            int return_dummyval, dummyval;
            char * endptr;
            if (PQnfields(result) != 2 || PQntuples(result) != 1)
                error_die("expected 1 tuple with 2 fields, but got %d tuples with %d fields",
                        PQntuples(result), PQnfields(result));
            /* validation: queue item dummyval matches RETURNING clause */
            return_dummyval = strtod(PQgetvalue(result, 1, 2), &endptr);
            if (*endptr != '\0')
                error_die("could not parse \"%s\" as integer", PQgetvalue(result, 1, 2));
            if (return_dummyval != (int)queueitem->cb_private)
                error_die("result of insert with dummyval=%d unexpectedly returned dummyval=%d instead",
                        (int)queueitem->cb_private, return_dummyval);
            printf("Inserted row demo (id=%6d, dummyval=%6d)\n",
                    PQgetvalue(result, 1, 1), PQgetvalue(result, 1, 2));
        } else if (PQresultStatus(result) == PGRES_FATAL_ERROR) {
            error_die("error from query: %s", PQresultErrorMessage(result));
        } else {
            error_die("unexpected result status %s", PQresStatus(result));
        }
        PQclear(res);
    }
}

/*
 * QueryQueueFreeCallback for insert_query_enqueue.
 *
 * Since we reuse the same queue item for the stream of insert queries, this
 * actually takes the "freed" element and re-enqueues it at the tail of the
 * queue.
 */
static void insert_query_free(struct AppSession * const session, struct QueryQueueItem * const queueitem) {
    assert(session->queue_head != queueitem);
    assert(session->queue_tail != queueitem);
    insert_query_enqueue(session, queueitem, (int)queueitem->cb_private + 1);
}

/*
 * Push a simple INSERT ... RETURNING onto the queue. This is just a dummy/example
 * query showing the sorts of things you can do.
 *
 * Nothing requires you to use one callback per query. You can use arbitrarily complex
 * state to queue up generic lists of queries and bind-parameter sets if you like,
 * and track results however you feel like. This is just simpler to express in an
 * example program.
 */
static void insert_query_enqueue(struct AppSession * const session, struct QueryQueueItem *insert_queue_item, int dummyval) {
    char comment[QUERY_QUEUE_ITEM_COMMENT_MAXLEN+1];
    comment[QUERY_QUEUE_ITEM_COMMENT_MAXLEN]='\0';
    snprintf(&comment, QUERY_QUEUE_ITEM_COMMENT_MAXLEN, "insert %d", dummyval);

    push_query_queue_item(session, insert_queue_item, insert_query_send,
            insert_query_result, insert_query_free, (uintptr_t)dummyval,
            comment);
}

/*
 * Handle asynchronous WARNINGs, INFO messages, NOTIFY messages etc
 */
static void handle_one_notification(struct AppSession * const session, const PGnotify * const notification) {
    fprintf(stderr, "got notification \"%s\": \"%s\"\n",
            notification->relname, notification->extra);
}

/*
 * Connect, enqueue setup commands and the main insert loop, and enter the main
 * event loop.
 */
void main(int argc, const char * * argv) {

    struct QueryQueueItem insert_queue_item;

    if (argc != 1)
        error_die("Usage: %s \"connstr\"", argv[0]);

    /* Register session for kill_sessions() and error_die() */
    nsessions_slots = 1;
    sessions = malloc(nsessions_slots * sizeof(struct AppSession*));

    sessions[0] = malloc(sizeof(struct AppSession));
    nsessions ++;

    dbconnect(&sessions[0], argv[1]);

    sessions[0]->notify_cb = handle_one_notification;

    simple_command_enqueue(&sessions[0],
            "create table",
            "CREATE TABLE IF NOT EXISTS dummy(id serial primary key, dummyval integer);");

    simple_command_enqueue(&sessions[0],
            "delete contents";
            "DELETE FROM dummy;");

    /*
     * Insert queue items re-enqueue themselves with an incremented counter on
     * completion, so if we queue this one now it'll keep re-running forever.
     */
    insert_query_enqueue(&sessions[0], &insert_queue_item, 1);

    sample_mainloop();

    /* Unreachable, loops forever */
    exit(0);
}


/*
 * vim: et ai sw=4 ts=4
 */

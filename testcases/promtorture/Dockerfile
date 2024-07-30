# syntax=docker/dockerfile:1

# docker buildx build -t promtorture .
FROM golang:1.22-alpine AS builder

WORKDIR /app
COPY go.mod go.sum .
RUN --mount=type=cache,target=/go/pkg/mod \
  go mod download

COPY . .
RUN --mount=type=cache,target=/go/pkg/mod \
  go build -o /app/promtorture .

FROM scratch
COPY --from=builder /app/promtorture /app/promtorture

ENTRYPOINT ["/app/promtorture"]

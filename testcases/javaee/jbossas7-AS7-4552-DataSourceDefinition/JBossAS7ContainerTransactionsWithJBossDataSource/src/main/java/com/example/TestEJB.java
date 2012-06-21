package com.example;

import javax.ejb.*;
import javax.inject.Inject;
import javax.inject.Named;
import javax.persistence.EntityManager;
import javax.persistence.Query;

/**
 * This test demonstrates that the container executes a method that requires
 * a transaction in a context where no transaction is actually running.
 * 
 * @author craig
 */
@Named
@Stateless
@TransactionManagement(TransactionManagementType.CONTAINER) // Default, but make it explicit
public class TestEJB {

    @Inject EntityManager em;
    @Inject MandatoryTransactionEJB txEJB;
    
    /**
     * Ask the container to make sure there's a transaction open and do some
     * work that can only be done within an open transaction.
     * 
     * This method will throw if there isn't really a transaction open.
     * 
     * @return true if test succeeded
     */
    @TransactionAttribute(TransactionAttributeType.REQUIRED)
    public boolean reallyInTransaction() {
        // Demonstrate the the container thinks we're in a transaction.
        // This call must fail if there's no transaction.
        txEJB.containerThinksWeAreInTransaction();
        
        
        // In PostgreSQL, a cursor may only be created within
        // an explicit transaction block. This provides us with one
        // handy way to prove we're within an explicit transaction.
        // 
        // We use a pre-defined system table just so we don't need any
        // DDL for this test.
        //
        // This test will fail here because we're not actually within a
        // transaction at all, despite the container telling us we are.
        //
        Query q = em.createNativeQuery("DECLARE test_cursor CURSOR FOR SELECT relname FROM pg_catalog.pg_class");
        q.executeUpdate();
        // If that worked, we can now fetch from the cursor. Fetch and discard
        // the results.
        q = em.createNativeQuery("FETCH ALL FROM test_cursor;");
        q.getResultList();
        
        return true;
    }
    
    
}

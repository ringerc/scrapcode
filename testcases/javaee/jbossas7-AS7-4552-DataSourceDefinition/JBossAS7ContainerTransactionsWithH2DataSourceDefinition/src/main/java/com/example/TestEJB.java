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
        
        // We can't use the cursor test trick we used in PostgreSQL
        // to test H2. Instead, rely on the fact that a SAVEPOINT can't
        // be created then rolled back to without an open transaction.
        
        Query q = em.createNativeQuery("SAVEPOINT test_tx_sp;");
        q.executeUpdate();
        // If that worked, we can now fetch from the cursor. Fetch and discard
        // the results.
        q = em.createNativeQuery("ROLLBACK TO SAVEPOINT test_tx_sp;");
        q.executeUpdate();
        
        return true;
    }
    
    
}

/*
 */
package com.example;

import javax.ejb.Stateless;
import javax.ejb.TransactionAttribute;
import javax.ejb.TransactionAttributeType;
import javax.ejb.TransactionManagement;
import javax.ejb.TransactionManagementType;

/**
 * The only purpose of this EJB is to provide a method that throws
 * if no transaction is active.
 * 
 * @author craig
 */
@Stateless
@TransactionManagement(TransactionManagementType.CONTAINER)
public class MandatoryTransactionEJB {
    
    /**
     * This method may only execute if the container thinks we're already within
     * a valid transaction. It won't create a transaction, so this method must
     * fail if no transaction already exists.
     * 
     * The container will throw if it doesn't think there's a transaction
     * open.
     * 
     * @return True
     */
    @TransactionAttribute(TransactionAttributeType.MANDATORY)
    public boolean containerThinksWeAreInTransaction() {
        return true;
    }
}

package com.example;

import javax.annotation.sql.DataSourceDefinition;
import javax.ejb.Stateless;
import javax.enterprise.inject.Produces;
import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

/**
 * Map a persistence unit to an injection point.
 * 
 * See README for test details.
 * 
 * @author craig
 */
@Stateless
public class Database {

    @PersistenceContext(name="test-PU")
    @Produces
    protected static EntityManager em;
    
}

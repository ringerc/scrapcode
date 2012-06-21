package com.example;

import javax.annotation.sql.DataSourceDefinition;
import javax.ejb.Stateless;
import javax.enterprise.inject.Produces;
import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

/**
 * Provide CDI injection of a persistence unit that uses a container
 * provided data source.
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

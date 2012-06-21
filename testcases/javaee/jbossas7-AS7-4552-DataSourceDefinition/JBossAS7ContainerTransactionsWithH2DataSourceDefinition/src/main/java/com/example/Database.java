package com.example;

import javax.annotation.sql.DataSourceDefinition;
import javax.ejb.Stateless;
import javax.enterprise.inject.Produces;
import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

/**
 * Define an application scoped datasource for testing purposes.
 * 
 * Note that we must use a DataSource; we can't supply a JDBC driver
 * class name and let JBoss AS wrap it in its own datasource like we do when we
 * create a data source via jboss-cli.
 * 
 * See README for test details.
 * 
 * @author craig
 */
@Stateless
@DataSourceDefinition(
        name="java:app/test-ds",
        className="org.h2.jdbcx.JdbcDataSource",
        url="jdbc:h2:mem:"        
)
public class Database {

    @PersistenceContext(name="test-PU")
    @Produces
    protected static EntityManager em;
    
}

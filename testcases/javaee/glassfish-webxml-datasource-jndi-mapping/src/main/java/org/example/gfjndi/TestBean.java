
package org.example.gfjndi;

import javax.enterprise.context.RequestScoped;
import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

/**
 *
 * @author craig
 */
@RequestScoped
public class TestBean {
    
    @PersistenceContext
    private EntityManager em;
    
    public String getHello() {
        return "hello";
    }
    
}

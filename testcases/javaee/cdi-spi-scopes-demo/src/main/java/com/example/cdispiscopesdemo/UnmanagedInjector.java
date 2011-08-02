package com.example.cdispiscopesdemo;

import java.util.Set;
import java.util.logging.Logger;
import javax.enterprise.context.spi.CreationalContext;
import javax.enterprise.inject.spi.Bean;
import javax.enterprise.inject.spi.BeanManager;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.ws.rs.WebApplicationException;

/**
 * This class is instantiated manually, not via CDI. It is
 * responsible for creating and injecting a bean using the 
 * CDI SPI.
 * 
 * @author Craig
 */
public class UnmanagedInjector {
    
    private static final Logger logger = Logger.getLogger(UnmanagedInjector.class.getName());
    
    private final BeanManager bm;
    private CreationalContext ctx;
    private final Bean<CustomInjectedBean> bean;
    
    public UnmanagedInjector() throws NamingException {
        bm = (BeanManager) InitialContext.doLookup("java:comp/BeanManager");
        Set<Bean<?>> beans = bm.getBeans(CustomInjectedBean.class);
        if (beans.size() != 1) {
            throw new IllegalStateException("Exactly 1 injection candidate of type CustomInjectedBean expected; " + beans.size() + " found");
        }
        bean = (Bean<CustomInjectedBean>)beans.iterator().next();
        ctx = bm.createCreationalContext(bean);
    }
    
    public CustomInjectedBean makeCustomInjectedBean() {
        return (CustomInjectedBean) bean.create(ctx);
    }
    
    public void releaseCustomInjectedBean(CustomInjectedBean instance) {
        bean.destroy(instance, ctx);
    }

}

package com.example.cdispiscopesdemo;

import com.example.cdispiscopesdemo.ClientAddressProducer.ClientAddress;
import java.util.UUID;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import javax.enterprise.context.RequestScoped;
import javax.faces.context.FacesContext;
import javax.inject.Inject;
import sun.misc.UUDecoder;

/**
 * This bean is instantiated using the CDI SPI, invoked manually from an
 * unmanaged bean.
 * 
 * @author Craig
 */
@RequestScoped
public class CustomInjectedBean {
    
    private static final Logger logger = Logger.getLogger(CustomInjectedBean.class.getName());
    
    @Inject @ClientAddress private String clientAddress;
    @Inject private RequestBean req;
    @Inject private SessionBean sess;
    @Inject private AppBean app;
    
   
    private final UUID uuid = UUID.randomUUID();
    
    public UUID getUUID() { 
        return uuid; 
    }
    
    @PostConstruct
    void postconstruct() {
        logger.log(Level.INFO, "postconstruct called on {0}", this);
    }
    
    @PreDestroy
    void predestroy() {
        logger.log(Level.INFO, "predestroy called on {0}", this);
    }
    
    void report(String msg) {
        logger.log(Level.INFO, "report called with msg: '{'0'}' on client '{'1'}'{0} from client {1}", new Object[]{msg, clientAddress});
        logger.log(Level.INFO, "CI bean is: {0}", uuid);
    }
    
    RequestBean getRequestBean() {
        return req;
    }
    
    SessionBean getSessionBean() {
        return sess;
    }
    
    AppBean getAppBean() {
        return app;
    }
    
}

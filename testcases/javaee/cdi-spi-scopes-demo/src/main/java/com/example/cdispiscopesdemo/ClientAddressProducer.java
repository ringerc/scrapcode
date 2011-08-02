package com.example.cdispiscopesdemo;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import javax.enterprise.inject.Produces;
import javax.faces.context.FacesContext;
import javax.inject.Qualifier;
import javax.servlet.http.HttpServletRequest;

/**
 * Provides a CDI producer for client addresses with an interface independent
 * of framework.
 * 
 * @author Craig
 */
public class ClientAddressProducer {
    
    /**
     * A CDI qualifier that identifies a producer as returning a client address,
     * usually IPv4 or IPv6.
     * 
     * @author Craig
     */
    @Qualifier
    @Retention(RetentionPolicy.RUNTIME)
    @java.lang.annotation.Target({ElementType.FIELD, ElementType.METHOD, ElementType.TYPE})
    public static @interface ClientAddress {}
    
    /**
     * Return the IP address of the client on the remote end of the current
     * request.
     * 
     * The address is returned as a string representation, and may be an IPv4
     * address, IPv6 address, or potentially any other transport the HTTP server
     * supports.
     * 
     * @return Client address
     */
    @Produces @ClientAddress
    public static String getClientAddress() {
        HttpServletRequest req = (HttpServletRequest)FacesContext.getCurrentInstance().getExternalContext().getRequest();
        return req.getRemoteAddr();
    }
    
}

package com.mycompany.embeddeddemo;

import javax.enterprise.context.RequestScoped;
import javax.ws.rs.GET;
import javax.ws.rs.Path;

/**
 * JAX-RS test class.
 * 
 * @author Craig
 */
@RequestScoped
@Path("/test")
public class TestClass {
    
    @GET
    public String getMe() {
        return "Hello World";
    }

}

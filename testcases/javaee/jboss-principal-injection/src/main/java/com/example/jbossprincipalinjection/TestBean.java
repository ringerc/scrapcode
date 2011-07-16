package com.example.jbossprincipalinjection;

import java.security.Principal;
import javax.enterprise.context.RequestScoped;
import javax.inject.Inject;
import javax.inject.Named;

@Named
@RequestScoped
public class TestBean {
    
    @Inject
    private Principal principal;
    
    public String getPrincipal() {
        return principal.toString();
    }
}
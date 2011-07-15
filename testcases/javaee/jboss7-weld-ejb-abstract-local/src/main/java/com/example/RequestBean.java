package com.example;

import javax.enterprise.context.RequestScoped;
import javax.inject.Inject;
import javax.inject.Named;

@Named
@RequestScoped
public class RequestBean {
    
    @Inject ConcreteEJBSubclass ejb;
    
    public String getSomething() {
        return ejb.doSomething();
    }
    
}

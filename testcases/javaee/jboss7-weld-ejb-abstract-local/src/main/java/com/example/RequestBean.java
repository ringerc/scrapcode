package com.example;

import javax.enterprise.context.RequestScoped;
import javax.inject.Inject;
import javax.inject.Named;

@Named
@RequestScoped
public class RequestBean {
    
    @Inject ConcreteEJBSubclass ejb;
    
    public String getMsg() {
        return ejb.getMsg();
    }
    
    public String getSomething() {
        if (ejb.doSomething() == null) {
            return "Result of doSomething was null as expected";
        } else {
            return "WTF? Result of doSomething non-null and not exception";
        }
    }
}

package com.example;

import javax.ejb.Stateless;

@Stateless
public class ConcreteEJBSubclass extends AbstractBase {
    
    @Override
    public String getMsg() {
        return "Hello";
    }
    
}

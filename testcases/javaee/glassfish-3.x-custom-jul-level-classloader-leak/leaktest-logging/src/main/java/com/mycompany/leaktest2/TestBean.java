package com.mycompany.leaktest2;

import java.util.logging.Logger;
import javax.annotation.ManagedBean;
import javax.faces.bean.RequestScoped;

public class TestBean {
    
    public String getHello() {
        // This will cause CustomLevel to be leaked, and thus the whole classloader
        // including any static data held by classes.
        Logger.getLogger(TestBean.class.getName()).log(CustomLevel.CUSTOM_LEVEL, "Sending custom log message");
        return "Hello";
    }
    
    
}

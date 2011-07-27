package com.example.cdispiscopesdemo;

import com.example.cdispiscopesdemo.ClientAddressProducer.ClientAddress;
import java.util.Random;
import java.util.UUID;
import javax.enterprise.context.RequestScoped;
import javax.inject.Inject;
import javax.inject.Named;

@Named
@RequestScoped
public class RequestBean extends BaseBean {
    
    @Override
    protected void ensureSameObject(CustomInjectedBean ci) {
        if (!ci.getRequestBean().getUUID().equals(this.getUUID())) {
            throw new IllegalStateException("Manually injected instance of RequestBean must be same as container-injected instance");
        }
    }
    
}

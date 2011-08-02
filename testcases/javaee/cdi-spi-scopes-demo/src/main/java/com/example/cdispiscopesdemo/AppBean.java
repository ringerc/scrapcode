package com.example.cdispiscopesdemo;

import com.example.cdispiscopesdemo.ClientAddressProducer.ClientAddress;
import java.io.Serializable;
import javax.enterprise.context.ApplicationScoped;
import javax.inject.Inject;
import javax.inject.Named;

@Named
@ApplicationScoped
public class AppBean extends BaseBean implements Serializable {

    private static final long serialVersionUid = 1L;
    
    @Override
    protected void ensureSameObject(CustomInjectedBean ci) {
        if (!ci.getAppBean().getUUID().equals(this.getUUID())) {
            throw new IllegalStateException("Manually injected instance of RequestBean must be same as container-injected instance");
        }
    }
    
}

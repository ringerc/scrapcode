package com.example.cdispiscopesdemo;

import com.example.cdispiscopesdemo.ClientAddressProducer.ClientAddress;
import java.io.Serializable;
import javax.enterprise.context.SessionScoped;
import javax.inject.Inject;
import javax.inject.Named;

@Named
@SessionScoped
public class SessionBean extends BaseBean implements Serializable {

    private static final long serialVersionUid = 1L;
        
    @Override
    protected void ensureSameObject(CustomInjectedBean ci) {
        if (!ci.getSessionBean().getUUID().equals(this.getUUID())) {
            throw new IllegalStateException("Manually injected instance of RequestBean must be same as container-injected instance");
        }
    }
    
}

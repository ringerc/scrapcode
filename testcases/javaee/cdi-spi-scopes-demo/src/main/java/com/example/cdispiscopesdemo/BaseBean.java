package com.example.cdispiscopesdemo;

import com.example.cdispiscopesdemo.ClientAddressProducer.ClientAddress;
import java.util.UUID;
import javax.inject.Inject;

public abstract class BaseBean {
    
    @Inject @ClientAddress private String clientAddress;
    
    private final UUID uuid = UUID.randomUUID();
    
    public String getClientAddress() {
        return clientAddress;
    }
    
    public UUID getUUID() {
        return uuid;
    }
    
    public String getCustomInjectorResult() {
        UnmanagedInjector ij = new UnmanagedInjector();
        CustomInjectedBean ci = ij.makeCustomInjectedBean();
        try {
            ci.report(clientAddress);
            ensureSameObject(ci);
            return "Success. The CI bean has UUID " + ci.getUUID();
        } finally {
            ij.releaseCustomInjectedBean(ci);
        }
    }
    
    protected abstract void ensureSameObject(CustomInjectedBean ci) throws IllegalStateException;
        
}

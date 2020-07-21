import javax.net.ssl.SSLContext;
import java.security.NoSuchAlgorithmException;
import javax.net.ssl.SSLSocketFactory;
import java.security.KeyManagementException;

public class SSLContextInfo {

    public static void main(String[] args)
    {
        int i, c;
        /* Try to establish a TLSv1 SSLContext */
        String[] protos = {"TLS", "TLSv1", "TLSv1.1", "SSLv3"};
        System.out.println("SSLContext creation attempts:");
        for (i = 0; i < protos.length; i++) {
            try {
                SSLContext ctx = SSLContext.getInstance(protos[i]);
                System.out.println("  " + protos[i] + " created: " + ctx.toString());
                System.out.println("    Provider: " + ctx.getProvider().toString());
                System.out.println("    Protocol: " + ctx.getProvider());
                try {
                    ctx.init(null, null, null);
                    System.out.println("    SSLContext.init(): ok");
                    SSLSocketFactory sf = ctx.getSocketFactory();
                    System.out.println("    SocketFactory: created: " + sf.toString());
                    String[] default_ciphers = sf.getDefaultCipherSuites();
                    System.out.println("    Default ciphers:");
                    for (c = 0; c < default_ciphers.length; c++) {
                        System.out.println("      " + default_ciphers[c]);
                    }
                    String[] supported_ciphers = sf.getSupportedCipherSuites();
                    System.out.println("    Supported ciphers:");
                    for (c = 0; c < supported_ciphers.length; c++) {
                        System.out.println("      " + supported_ciphers[c]);
                    }
                } catch (KeyManagementException kmx) {
                    System.out.println("    SSLContext.init(): failed: " + kmx.toString());
                }
            } catch (NoSuchAlgorithmException ex) {
                System.out.println("  " + protos[i] + " failed: " + ex.toString());
            }
        }
    }
}

// vim: ts=4 sw=4 et ai

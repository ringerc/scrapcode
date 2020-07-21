import java.security.Security;
import java.security.Provider;

public class JSSInfo {

    public static void main(String[] args)
    {
        /* JSA providers */
        Provider[] providers = Security.getProviders();
        System.out.println("JSA providers:");
        int i;
        for (i = 0; i < providers.length; i++)
        {
            System.out.println("  " + i + " " + providers[i].toString());
        }
        System.out.println("\n");
    }
}

// vim: ts=4 sw=4 et ai

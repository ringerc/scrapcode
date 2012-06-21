package id.au.ringerc.testcase.as7.eclipselink;

import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.transaction.TransactionManager;

import org.eclipse.persistence.platform.server.jboss.JBossPlatform;
import org.eclipse.persistence.sessions.DatabaseSession;
import org.eclipse.persistence.transaction.jboss.JBossTransactionController;

public class JBossAS7ServerPlatform extends JBossPlatform {

    public JBossAS7ServerPlatform(DatabaseSession newDatabaseSession) {
        super(newDatabaseSession);
    }

    @Override
    public Class<?> getExternalTransactionControllerClass() {
        return JBossAS7TransactionController.class;
    }

    public static class JBossAS7TransactionController extends JBossTransactionController {
        @Override
        protected TransactionManager acquireTransactionManager() throws Exception {
            try {
                return InitialContext.doLookup("java:jboss/TransactionManager");
            } catch (NamingException ex) {
                return super.acquireTransactionManager();
            }
        }
    }

}

package au.id.ringerc.testcases.eclipselink_as7.metamodel;

import junit.framework.Assert;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(Arquillian.class)
public class PersistenceUnitTest_createEntityManagerTest extends PersistenceUnitTestBase {

	// Calling createEntityManager DOES populate the metamodel
	@Test
	public void metamodelPopulatedAfterEntityManagerCreation() {
		expectNullBefore();
		emf.createEntityManager();
		expectNotNullAfter();
	}
	
	@Deployment
	public static WebArchive createDeployment() {
		return PersistenceUnitTestBase.createDeployment();
	}
}

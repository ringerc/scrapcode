package au.id.ringerc.testcases.eclipselink_as7.metamodel;

import junit.framework.Assert;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(Arquillian.class)
public class PersistenceUnitTest_getPropertiesTest extends PersistenceUnitTestBase {

	
	@Test
	public void metamodelPopulatedAfterPropertiesAccess() {
		expectNullBefore();
		emf.getProperties();
		// Calling getProperties does NOT populate the metamodel
		expectNullAfter();
	}
	
	@Deployment
	public static WebArchive createDeployment() {
		return PersistenceUnitTestBase.createDeployment();
	}
}

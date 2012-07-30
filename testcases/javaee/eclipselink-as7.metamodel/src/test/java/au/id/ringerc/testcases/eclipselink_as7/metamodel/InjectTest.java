package au.id.ringerc.testcases.eclipselink_as7.metamodel;

import java.io.File;
import au.id.ringerc.testcases.eclipselink_as7.metamodel.DummyEntity_;

import javax.ejb.Stateless;
import javax.inject.Inject;
import javax.persistence.EntityManager;

import junit.framework.Assert;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.ShrinkWrap;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Test;
import org.junit.experimental.theories.suppliers.TestedOn;
import org.junit.runner.RunWith;
import com.sun.crypto.provider.ARCFOURCipher;

@RunWith(Arquillian.class)
public class InjectTest extends TestBase {

	@Inject
	private EntityManager em;
	
	@Test
	public void metamodelPopulatedAfter() {
		expectNullBefore();
		em.isOpen();
		expectNotNullAfter();
	}
	
	@Deployment
	public static WebArchive createDeployment() {
		return TestBase.makeDeployment("injectTest.war").addClass(InjectTestDBProvider.class);
	}
	
}

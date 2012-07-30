package au.id.ringerc.testcases.eclipselink_as7.metamodel;

import javax.annotation.Resource;
import au.id.ringerc.testcases.eclipselink_as7.metamodel.DummyEntity_;
import javax.ejb.Stateless;
import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

import junit.framework.Assert;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(Arquillian.class)
public class PersistenceContextTest extends TestBase {

	@PersistenceContext(name="eclipselink-as7.metamodel")
	private EntityManager em;

	@Test
	public void metamodelPopulatedAfter() {
		expectNullBefore();
		em.isOpen();
		expectNotNullAfter();
	}

	@Deployment
	public static WebArchive createDeployment() {
		return TestBase.makeDeployment("persistencecontexttest.war");
	}
	
}

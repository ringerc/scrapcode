package au.id.ringerc.testcases.eclipselink_as7.metamodel;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
import javax.persistence.PersistenceContext;
import javax.persistence.PersistenceUnit;
import au.id.ringerc.testcases.eclipselink_as7.metamodel.DummyEntity_;

import junit.framework.Assert;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;

public class PersistenceUnitTestBase extends TestBase {

	@PersistenceUnit(name="eclipselink-as7.metamodel")
	protected EntityManagerFactory emf;
	
	protected static WebArchive createDeployment() {
		return TestBase.makeDeployment("persistenceunittest.war").addClass(PersistenceUnitTestBase.class);
	}
}
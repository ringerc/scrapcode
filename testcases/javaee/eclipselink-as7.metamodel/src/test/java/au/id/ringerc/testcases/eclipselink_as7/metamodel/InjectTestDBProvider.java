package au.id.ringerc.testcases.eclipselink_as7.metamodel;

import javax.enterprise.inject.Produces;
import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

public class InjectTestDBProvider {
	
	@PersistenceContext(name="eclipselink-as7.metamodel")
	@Produces
	protected EntityManager em;

}
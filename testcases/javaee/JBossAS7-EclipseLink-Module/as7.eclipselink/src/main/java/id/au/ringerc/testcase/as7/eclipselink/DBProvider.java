package id.au.ringerc.testcase.as7.eclipselink;

import javax.enterprise.inject.Produces;
import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

public class DBProvider {
	
	@PersistenceContext(name="as7.eclipselink")
	@Produces
	protected EntityManager em;

}

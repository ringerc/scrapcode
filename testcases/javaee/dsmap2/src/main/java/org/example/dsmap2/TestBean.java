import javax.faces.bean.ManagedBean;
import javax.faces.bean.RequestScoped;
import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;

@ManagedBean
@RequestScoped
public class TestBean {
    
    @PersistenceContext
    private EntityManager em;
    
    public String getHello() {
        return "Hello";
    }
    
    public String getEm() {
        return em == null ? "null" : em.toString();
    }
    
}

package id.au.ringerc.testcase.as7.eclipselink.entities;

import java.io.Serializable;
import java.lang.Integer;
import java.lang.String;
import javax.persistence.*;

@Entity
public class DummyEntity implements Serializable {

	   
	@Id
	private Integer id;
	private String dummy;
	private static final long serialVersionUID = 1L;

	public DummyEntity() {
		super();
	}
	
	public DummyEntity(Integer id) {
		this.id = id;
	}
	
	public Integer getId() {
		return this.id;
	}

	public void setId(Integer id) {
		this.id = id;
	}   
	public String getDummy() {
		return this.dummy;
	}

	public void setDummy(String dummy) {
		this.dummy = dummy;
	}
   
}

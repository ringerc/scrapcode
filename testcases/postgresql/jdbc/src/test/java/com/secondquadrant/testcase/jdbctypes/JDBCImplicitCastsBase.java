package com.secondquadrant.testcase.jdbctypes;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

@Ignore
public class JDBCImplicitCastsBase {
	
	private static final String
		dummyjson = "{\"name\": \"value\"}",
		dummytext = "{\"name\": \"value\"}",
		dummyxml  = "<element/>";
	
	protected static final String 
		url = System.getProperty("test.url"),
		username = System.getProperty("test.username"),
		password = System.getProperty("test.password");
	
	protected static boolean allowImplicitCasts;

	private Connection conn;

	private void execSetString(String sql, String param) throws SQLException {
		PreparedStatement stm = conn.prepareStatement(sql);
		stm.setString(1, param);
		stm.execute();
		stm.close();
	}
	
	private void execSetObject(String sql, String param) throws SQLException {
		PreparedStatement stm = conn.prepareStatement(sql);
		stm.setObject(1, param);
		stm.execute();
		stm.close();
	}
	
	private void execSetObject(String sql, String param, int paramType) throws SQLException {
		PreparedStatement stm = conn.prepareStatement(sql);
		stm.setObject(1, param, paramType);
		stm.execute();
		stm.close();
	}
	
	@BeforeClass
	public static void setUpClass() throws SQLException {
		if (url == null) {
			throw new RuntimeException("System property test.url is not set");
		}
		
		Connection c;
		try {
			c = DriverManager.getConnection(url, username, password);
		} catch (SQLException ex) {
			if (username == null || password == null) {
				System.err.println("Warning: System properies test.username and/or test.password are not set. Connection failed using defaults with: " + ex);
			}
			throw new RuntimeException(ex);
		}
		
		// Quick and dirty JDBC coding; let the driver and GC handle statement cleanup.
		PreparedStatement stm = c.prepareStatement(
				"CREATE OR REPLACE FUNCTION testjson(json) RETURNS json " +
				"LANGUAGE sql AS $$" +
				"SELECT $1;" +
				"$$;" +
				
				"CREATE OR REPLACE FUNCTION testtext(text) RETURNS text " +
				"LANGUAGE sql AS $$" +
				"SELECT $1;" +
				"$$;"+
				
				"CREATE OR REPLACE FUNCTION testxml(xml) RETURNS xml " +
				"LANGUAGE sql AS $$" +
				"SELECT $1;" +
				"$$;"+
				
				"CREATE OR REPLACE FUNCTION ambiguous(text) RETURNS text " +
				"LANGUAGE sql AS $$" +
				"SELECT $1;" +
				"$$;"+
				
				"CREATE OR REPLACE FUNCTION ambiguous(json) RETURNS text " +
				"LANGUAGE sql AS $$" +
				"SELECT $1::text;" +
				"$$;"
		);
		stm.execute();
		stm.close();
		c.close();
	}
	
	private void makeCastsImplicit() throws SQLException {
		PreparedStatement stm;
		
		// The XML casts already exist. Allow them to be invoked implicitly.
		stm = conn.prepareStatement(
				"UPDATE pg_catalog.pg_cast " +
				"SET castcontext = 'i' " +
				"WHERE casttarget = 'xml'::regtype " +
				"AND (castsource = 'text'::regtype OR castsource = 'varchar'::regtype);"
		);
		stm.execute();
		stm.close();
		
		// No casts exist for JSON in a default DB. Create them.
		// (Would need to test for this by doing the above query with a RETURNING clause
		// in production code, but this isn't).
		conn.prepareStatement("CREATE CAST (text AS json) WITHOUT FUNCTION AS IMPLICIT;").execute();
		conn.prepareStatement("CREATE CAST (varchar AS json) WITHOUT FUNCTION AS IMPLICIT;").execute();

	}
	
	@Before
	public void setUp() throws SQLException {
		conn = DriverManager.getConnection(url, username, password);
		conn.setAutoCommit(false);
		if (allowImplicitCasts) {
			makeCastsImplicit();
		}
	}
	
	@After
	public void tearDown() throws SQLException {
		conn.rollback();
		conn.close();
	}

	/* Tests of a function taking text */
	
	@Test
	public void testTextFuncSetObjectVarchar() throws SQLException {
		execSetObject("SELECT testtext(?)", dummytext, java.sql.Types.VARCHAR);
	}


	@Test
	public void testTextFuncSetObjectOther() throws SQLException {
		execSetObject("SELECT testtext(?)", dummytext, java.sql.Types.OTHER);
	}

	@Test
	public void testTextFuncSetObjectDetect() throws SQLException {
		execSetObject("SELECT testtext(?)", dummytext);
	}

	@Test
	public void testTextFuncSetString() throws SQLException {
		execSetString("SELECT testtext(?)", dummytext);
	}
	
	
	
	/* Test of a function taking json. Using json instead of xml as an example
	 * because Java has built-in SQL/XML support, so json is a better demo of the
	 * problems with Pg's type handling. The same occurs if you want to use XML
	 * as validated text-like fields rather than via SQL/XML. */
	

	@Test
	public void testJSONFuncSetObjectVarchar() throws SQLException {
		execSetObject("SELECT testjson(?)", dummyjson, java.sql.Types.VARCHAR);
	}


	@Test
	public void testJSONFuncSetObjectOther() throws SQLException {
		execSetObject("SELECT testjson(?)", dummyjson, java.sql.Types.OTHER);
	}

	@Test
	public void testJSONFuncSetObjectDetect() throws SQLException {
		execSetObject("SELECT testjson(?)", dummyjson);
	}

	@Test
	public void testJSONFuncSetString() throws SQLException {
		execSetString("SELECT testjson(?)", dummyjson);
	}
	
	
	
	/* Accessing xml fields as strings */
	

	@Test
	public void testXMLFuncSetObjectVarchar() throws SQLException {
		execSetObject("SELECT testxml(?)", dummyxml, java.sql.Types.VARCHAR);
	}


	@Test
	public void testXMLFuncSetObjectOther() throws SQLException {
		execSetObject("SELECT testxml(?)", dummyxml, java.sql.Types.OTHER);
	}

	@Test
	public void testXMLFuncSetObjectDetect() throws SQLException {
		execSetObject("SELECT testxml(?)", dummyxml);
	}

	@Test
	public void testXMLFuncSetString() throws SQLException {
		execSetString("SELECT testxml(?)", dummyxml);
	}
	
	@Test
	public void testXMLFuncSetXML() throws SQLException {
		execSetObject("SELECT testxml(?)", dummyxml, java.sql.Types.SQLXML);
	}
	
	
	/* 
	 * Type-based ambiguous function resolution. These will all resolve to the 'text' form
	 * because PgJDBC will treat java.lang.String as 'text'.
	 * 
	 * If you:
	 * 
	 * 
	 * then these tests will *fail*. I can't easily automate that in the test case
	 * because you need elevated rights to create the casts.
	 * 
	 */

	// I really expected this to fail with an ambiguous function resolution
	// error, but Pg doesn't seem to consider the json-accepting function
	// as a candidate.
	@Test
	public void testAmbiguousUntypedLiteral() throws SQLException {
		PreparedStatement stm = conn.prepareStatement(
				"SELECT ambiguous('some untyped literal')"
		);
		stm.execute();
		stm.close();
	}

	@Test
	public void testAmbiguousSetObjectVarchar() throws SQLException {
		execSetObject("SELECT ambiguous(?)", dummyjson, java.sql.Types.VARCHAR);
	}


	@Test
	public void testAmbiguousSetObjectOther() throws SQLException {
		execSetObject("SELECT ambiguous(?)", dummyjson, java.sql.Types.OTHER);
	}

	@Test
	public void testAmbiguousSetObjectDetect() throws SQLException {
		execSetObject("SELECT ambiguous(?)", dummyjson);
	}

	@Test
	public void testAmbiguousSetString() throws SQLException {
		execSetString("SELECT ambiguous(?)", dummyjson);
	}
	
	
}

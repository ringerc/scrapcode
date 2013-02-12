package com.secondquadrant.testcase.jdbctypes;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

import org.junit.BeforeClass;

public class ImplicitCastsTest extends JDBCImplicitCastsBase {

	static {
		allowImplicitCasts = true;
	}

	@BeforeClass
	public static void ensureSuperuser() throws SQLException {
		Connection c = DriverManager.getConnection(url, username, password);
		PreparedStatement stm = c.prepareStatement("SELECT current_setting('is_superuser');");
		stm.execute();
		ResultSet rs = stm.getResultSet();
		rs.next();
		if (rs.getString(1).equals("off")) {
			throw new RuntimeException("Cannot test with implicit casting; need to be connected as a superuser");
		}
		stm.close();
		c.close();
	}
}

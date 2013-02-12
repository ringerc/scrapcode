package com.secondquadrant.testcase.jdbctypes;

public class NoImplicitTest extends JDBCImplicitCastsBase {

	static {
		allowImplicitCasts = false;
	}
}
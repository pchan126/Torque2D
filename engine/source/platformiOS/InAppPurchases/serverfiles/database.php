<?php

	class Database extends PDO
	{
		static private $host = "xxxxxxxxx";
		static private $dbname = "xxxxxxxxxx";
		static private $user = "xxxxxxxxxx";
		static private $pass = "xxxxxxxxxx";
		
		private static $instance = null;
		
		static function get() {
			if (self::$instance!=null) return self::$instance;
		
			try {
				self::$instance = new
					Database("mysql:host=".self::$host.";dbname=".self::$dbname,self::$user, self::$pass);
				return self::$instance;
				}
			catch(PDOException $e) {
				print $e->getMessage();
				return null;
			}
		}
	}
?>
CREATE DATABASE IF NOT EXISTS campus_managment;
USE campus_managment;

SET FOREIGN_KEY_CHECKS = 0;

DROP TABLE IF EXISTS instructor_courses;
DROP TABLE IF EXISTS student_courses;
DROP TABLE IF EXISTS account_security;
DROP TABLE IF EXISTS login_attempts;
DROP TABLE IF EXISTS logs;
DROP TABLE IF EXISTS instructors;
DROP TABLE IF EXISTS students;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS courses;
DROP TABLE IF EXISTS majors;

SET FOREIGN_KEY_CHECKS = 1;

CREATE TABLE majors (
  major_id INT NOT NULL AUTO_INCREMENT,
  major_name VARCHAR(100) NOT NULL,
  PRIMARY KEY (major_id),
  UNIQUE KEY major_name (major_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO majors (major_id, major_name) VALUES
(1,'Computer Science'),
(2,'Electrical Engineering'),
(3,'Business Administration'),
(4,'Mechanical Engineering'),
(5,'Civil Engineering'),
(6,'Pharmacy'),
(7,'Marketing');

CREATE TABLE users (
  user_id INT NOT NULL AUTO_INCREMENT,
  username VARCHAR(50) NOT NULL,
  password_hash VARCHAR(255) NOT NULL,
  role VARCHAR(50) NOT NULL,
  major_id INT DEFAULT NULL,
  created_at TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (user_id),
  UNIQUE KEY username (username),
  KEY major_id (major_id),
  CONSTRAINT users_fk_major FOREIGN KEY (major_id) REFERENCES majors (major_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Test-only password hashes. They are not copied from the real database.
INSERT INTO users (user_id, username, password_hash, role, major_id) VALUES
(1350,'ci_admin','$2b$10$aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa','Admin',NULL),
(3001,'ci_student','$2b$10$aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa','Student',7),
(5001,'ci_ahmed','$2b$10$aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa','Instructor',6),
(5006,'ci_instructor2','$2b$10$aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa','Instructor',3);

CREATE TABLE courses (
  course_id INT NOT NULL AUTO_INCREMENT,
  course_name VARCHAR(50) NOT NULL,
  credit_hours INT NOT NULL,
  major_id INT DEFAULT NULL,
  PRIMARY KEY (course_id),
  KEY major_id (major_id),
  CONSTRAINT courses_fk_major FOREIGN KEY (major_id) REFERENCES majors (major_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO courses (course_id, course_name, credit_hours, major_id) VALUES
(1,'C++',3,1),(2,'Data Structures',3,1),(3,'Databases',3,1),
(4,'Operating Systems',3,1),(5,'Networks',3,1),
(6,'Circuit Analysis',3,2),(7,'Electromagnetics',3,2),
(8,'Digital Systems',3,2),(9,'Control Systems',3,2),
(10,'Accounting',3,3),(11,'Finance',3,3),(12,'Organizational Behavior',3,3),
(13,'Business Ethics',3,3),(14,'Thermodynamics',3,4),(15,'Fluid Mechanics',3,4),
(16,'Dynamics',3,4),(17,'Materials Science',3,4),(18,'Statics',3,5),
(19,'Structural Analysis',3,5),(20,'Geotechnical Engineering',3,5),(21,'Hydraulics',3,5),
(22,'Pharmacology',3,6),(23,'Pharmaceutics',3,6),(24,'Medicinal Chemistry',3,6),
(25,'Clinical Pharmacy',3,6),(26,'Principles of Marketing',3,7),
(27,'Consumer Behavior',3,7),(28,'Digital Marketing',3,7),(29,'Brand Management',3,7);

CREATE TABLE students (
  student_id INT NOT NULL,
  first_name VARCHAR(50) DEFAULT NULL,
  last_name VARCHAR(50) DEFAULT NULL,
  major_id INT DEFAULT NULL,
  PRIMARY KEY (student_id),
  KEY major_id (major_id),
  CONSTRAINT students_fk_user FOREIGN KEY (student_id) REFERENCES users (user_id),
  CONSTRAINT students_fk_major FOREIGN KEY (major_id) REFERENCES majors (major_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO students VALUES
(3001,'CI','Student',7);

CREATE TABLE instructors (
  instructor_id INT NOT NULL,
  first_name VARCHAR(50) NOT NULL,
  last_name VARCHAR(50) NOT NULL,
  major_id INT DEFAULT NULL,
  PRIMARY KEY (instructor_id),
  KEY major_id (major_id),
  CONSTRAINT instructors_fk_user FOREIGN KEY (instructor_id) REFERENCES users (user_id),
  CONSTRAINT instructors_fk_major FOREIGN KEY (major_id) REFERENCES majors (major_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO instructors VALUES
(5001,'CI','Instructor',6),
(5006,'CI','Instructor2',3);

CREATE TABLE account_security (
  user_id INT NOT NULL,
  failed_attempts INT DEFAULT 0,
  locked TINYINT(1) DEFAULT 0,
  locked_until DATETIME DEFAULT NULL,
  last_failed_attempt TIMESTAMP NULL DEFAULT NULL,
  last_login TIMESTAMP NULL DEFAULT NULL,
  PRIMARY KEY (user_id),
  CONSTRAINT account_security_fk_user FOREIGN KEY (user_id) REFERENCES users (user_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO account_security
(user_id, failed_attempts, locked, locked_until, last_failed_attempt, last_login)
VALUES
(1350,0,0,NULL,NULL,NULL),
(3001,0,0,NULL,NULL,NULL),
(5001,0,0,NULL,NULL,NULL),
(5006,0,0,NULL,NULL,NULL);

CREATE TABLE student_courses (
  student_id INT NOT NULL,
  course_id INT NOT NULL,
  grade INT DEFAULT NULL,
  PRIMARY KEY (student_id, course_id),
  KEY course_id (course_id),
  CONSTRAINT student_courses_fk_student FOREIGN KEY (student_id) REFERENCES students (student_id) ON DELETE CASCADE,
  CONSTRAINT student_courses_fk_course FOREIGN KEY (course_id) REFERENCES courses (course_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE instructor_courses (
  instructor_id INT NOT NULL,
  course_id INT NOT NULL,
  PRIMARY KEY (instructor_id, course_id),
  KEY course_id (course_id),
  CONSTRAINT instructor_courses_fk_instructor FOREIGN KEY (instructor_id) REFERENCES instructors (instructor_id) ON DELETE CASCADE,
  CONSTRAINT instructor_courses_fk_course FOREIGN KEY (course_id) REFERENCES courses (course_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO instructor_courses VALUES
(5006,10),(5006,12),(5001,22),(5001,25);

CREATE TABLE logs (
  log_id INT NOT NULL AUTO_INCREMENT,
  user_id INT DEFAULT NULL,
  message VARCHAR(255) DEFAULT NULL,
  log_time TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (log_id),
  KEY user_id (user_id),
  CONSTRAINT logs_fk_user FOREIGN KEY (user_id) REFERENCES users (user_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO logs (log_id, user_id, message) VALUES
(2,1350,'CI test log'),
(1,5001,'CI test Number 2');

CREATE TABLE login_attempts (
  attempt_id INT NOT NULL AUTO_INCREMENT,
  username_attempted VARCHAR(50) DEFAULT NULL,
  ip_address VARCHAR(45) DEFAULT NULL,
  success TINYINT(1) DEFAULT 0,
  attempt_time TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (attempt_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO login_attempts (username_attempted, ip_address, success) VALUES
('ci_admin','127.0.0.1',1),
('unknown_ci_user','127.0.0.1',0);

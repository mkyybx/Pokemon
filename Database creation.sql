CREATE DATABASE  IF NOT EXISTS `pokemon__new` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `pokemon__new`;
-- MySQL dump 10.13  Distrib 5.7.17, for Win64 (x86_64)
--
-- Host: 127.0.0.1    Database: pokemon__new
-- ------------------------------------------------------
-- Server version	5.7.11-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `initpokemon`
--

DROP TABLE IF EXISTS `initpokemon`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `initpokemon` (
  `idinitpokemon` int(11) NOT NULL,
  `name` varchar(45) DEFAULT NULL,
  `attackDamagel` int(11) DEFAULT NULL,
  `attackDamageh` int(11) DEFAULT NULL,
  `powerl` int(11) DEFAULT NULL,
  `powerh` int(11) DEFAULT NULL,
  `armor` int(11) DEFAULT NULL,
  `mResist` int(11) DEFAULT NULL,
  `critical` float DEFAULT NULL,
  `agile` int(11) DEFAULT NULL,
  `maxHP` int(11) DEFAULT NULL,
  `maxMP` int(11) DEFAULT NULL,
  `avoidance` float DEFAULT NULL,
  `rarity` float DEFAULT NULL,
  `type` int(11) DEFAULT NULL,
  PRIMARY KEY (`idinitpokemon`),
  UNIQUE KEY `idinitpokemon_UNIQUE` (`idinitpokemon`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `initpokemon`
--

LOCK TABLES `initpokemon` WRITE;
/*!40000 ALTER TABLE `initpokemon` DISABLE KEYS */;
/*!40000 ALTER TABLE `initpokemon` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `pokemon`
--

DROP TABLE IF EXISTS `pokemon`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pokemon` (
  `idpokemon` int(11) NOT NULL AUTO_INCREMENT,
  `data` varchar(4096) NOT NULL,
  PRIMARY KEY (`idpokemon`),
  UNIQUE KEY `idpokemon_UNIQUE` (`idpokemon`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `pokemon`
--

LOCK TABLES `pokemon` WRITE;
/*!40000 ALTER TABLE `pokemon` DISABLE KEYS */;
/*!40000 ALTER TABLE `pokemon` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-12-05 11:39:32

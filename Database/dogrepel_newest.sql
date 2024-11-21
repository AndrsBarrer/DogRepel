-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Nov 21, 2024 at 03:51 AM
-- Server version: 10.4.32-MariaDB
-- PHP Version: 8.0.30

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `dogrepel`
--

-- --------------------------------------------------------

--
-- Table structure for table `dogs`
--

CREATE TABLE `dogs` (
  `dog_id` int(11) NOT NULL,
  `mac` varchar(255) NOT NULL,
  `name` varchar(255) NOT NULL DEFAULT 'Dog',
  `breed` varchar(255) DEFAULT 'Dog',
  `age` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- --------------------------------------------------------

--
-- Table structure for table `dog_visits`
--

CREATE TABLE `dog_visits` (
  `visit_id` int(11) NOT NULL,
  `dog_name` varchar(255) NOT NULL,
  `station_id` int(11) NOT NULL,
  `location` varchar(255) NOT NULL,
  `visit_time` timestamp NULL DEFAULT current_timestamp(),
  `distance` float NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

-- --------------------------------------------------------

--
-- Table structure for table `rssi_categories`
--

CREATE TABLE `rssi_categories` (
  `category` varchar(15) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `threshold` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `rssi_categories`
--

INSERT INTO `rssi_categories` (`category`, `threshold`) VALUES
('HIGH', -30),
('LOW', -70),
('MEDIUM', -50);

-- --------------------------------------------------------

--
-- Table structure for table `stations`
--

CREATE TABLE `stations` (
  `station_id` int(11) NOT NULL,
  `mac` varchar(255) NOT NULL,
  `location` varchar(255) DEFAULT 'Location',
  `category` varchar(15) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_general_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `dogs`
--
ALTER TABLE `dogs`
  ADD PRIMARY KEY (`dog_id`);

--
-- Indexes for table `dog_visits`
--
ALTER TABLE `dog_visits`
  ADD PRIMARY KEY (`visit_id`);

--
-- Indexes for table `rssi_categories`
--
ALTER TABLE `rssi_categories`
  ADD PRIMARY KEY (`category`),
  ADD KEY `idx_category` (`category`);

--
-- Indexes for table `stations`
--
ALTER TABLE `stations`
  ADD PRIMARY KEY (`station_id`),
  ADD KEY `fk_category` (`category`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `dogs`
--
ALTER TABLE `dogs`
  MODIFY `dog_id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=11;

--
-- AUTO_INCREMENT for table `dog_visits`
--
ALTER TABLE `dog_visits`
  MODIFY `visit_id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4178;

--
-- AUTO_INCREMENT for table `stations`
--
ALTER TABLE `stations`
  MODIFY `station_id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=15;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `stations`
--
ALTER TABLE `stations`
  ADD CONSTRAINT `fk_category` FOREIGN KEY (`category`) REFERENCES `rssi_categories` (`category`) ON UPDATE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;

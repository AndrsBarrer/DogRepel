-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Sep 09, 2024 at 05:54 AM
-- Server version: 8.4.0
-- PHP Version: 8.2.12

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
  `dog_id` int NOT NULL,
  `name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  `breed` varchar(255) DEFAULT NULL,
  `age` int DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `dogs`
--

INSERT INTO `dogs` (`dog_id`, `name`, `breed`, `age`) VALUES
(1, 'Maya', 'Chihuahua', 1);

-- --------------------------------------------------------

--
-- Table structure for table `dog_settings`
--

CREATE TABLE `dog_settings` (
  `dog_id` int NOT NULL,
  `alert_distance` float NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- --------------------------------------------------------

--
-- Table structure for table `dog_visits`
--

CREATE TABLE `dog_visits` (
  `visit_id` int NOT NULL,
  `dog_id` int NOT NULL,
  `station_id` int NOT NULL,
  `visit_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `distance` float NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `dog_visits`
--

INSERT INTO `dog_visits` (`visit_id`, `dog_id`, `station_id`, `visit_time`, `distance`) VALUES
(1, 1, 1, '2024-09-09 00:34:08', 2.4);

-- --------------------------------------------------------

--
-- Table structure for table `images`
--

CREATE TABLE `images` (
  `image_id` int NOT NULL,
  `visit_id` int NOT NULL,
  `image_path` varchar(255) NOT NULL,
  `capture_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- --------------------------------------------------------

--
-- Table structure for table `stations`
--

CREATE TABLE `stations` (
  `station_id` int NOT NULL,
  `location` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `stations`
--

INSERT INTO `stations` (`station_id`, `location`) VALUES
(1, 'Living Room');

-- --------------------------------------------------------

--
-- Table structure for table `station_settings`
--

CREATE TABLE `station_settings` (
  `station_id` int NOT NULL,
  `sensitivity` int NOT NULL,
  `camera_activation` tinyint(1) NOT NULL DEFAULT '1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `dogs`
--
ALTER TABLE `dogs`
  ADD PRIMARY KEY (`dog_id`);

--
-- Indexes for table `dog_settings`
--
ALTER TABLE `dog_settings`
  ADD PRIMARY KEY (`dog_id`);

--
-- Indexes for table `dog_visits`
--
ALTER TABLE `dog_visits`
  ADD PRIMARY KEY (`visit_id`),
  ADD KEY `dog_id` (`dog_id`),
  ADD KEY `station_id` (`station_id`);

--
-- Indexes for table `images`
--
ALTER TABLE `images`
  ADD PRIMARY KEY (`image_id`),
  ADD KEY `visit_id` (`visit_id`);

--
-- Indexes for table `stations`
--
ALTER TABLE `stations`
  ADD PRIMARY KEY (`station_id`);

--
-- Indexes for table `station_settings`
--
ALTER TABLE `station_settings`
  ADD PRIMARY KEY (`station_id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `dogs`
--
ALTER TABLE `dogs`
  MODIFY `dog_id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT for table `dog_visits`
--
ALTER TABLE `dog_visits`
  MODIFY `visit_id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT for table `images`
--
ALTER TABLE `images`
  MODIFY `image_id` int NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `stations`
--
ALTER TABLE `stations`
  MODIFY `station_id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT for table `station_settings`
--
ALTER TABLE `station_settings`
  MODIFY `station_id` int NOT NULL AUTO_INCREMENT;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `dog_settings`
--
ALTER TABLE `dog_settings`
  ADD CONSTRAINT `dog_settings_ibfk_1` FOREIGN KEY (`dog_id`) REFERENCES `dogs` (`dog_id`);

--
-- Constraints for table `dog_visits`
--
ALTER TABLE `dog_visits`
  ADD CONSTRAINT `dog_visits_ibfk_1` FOREIGN KEY (`dog_id`) REFERENCES `dogs` (`dog_id`),
  ADD CONSTRAINT `dog_visits_ibfk_2` FOREIGN KEY (`station_id`) REFERENCES `stations` (`station_id`);

--
-- Constraints for table `images`
--
ALTER TABLE `images`
  ADD CONSTRAINT `images_ibfk_1` FOREIGN KEY (`visit_id`) REFERENCES `dog_visits` (`visit_id`);

--
-- Constraints for table `station_settings`
--
ALTER TABLE `station_settings`
  ADD CONSTRAINT `station_settings_ibfk_1` FOREIGN KEY (`station_id`) REFERENCES `stations` (`station_id`);
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;

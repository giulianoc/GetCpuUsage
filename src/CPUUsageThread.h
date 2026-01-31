/*
Copyright (C) Giuliano Catrambone (giulianocatrambone@gmail.com)

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/

#pragma once
#include <cstdint>
#include <spdlog/logger.h>
#include <thread>

class CPUUsageThread
{
public:
	explicit CPUUsageThread(const int16_t cpuStatsUpdateIntervalInSeconds = 10,
		const std::shared_ptr<spdlog::logger>& logger = nullptr):
		_running(false), _stopSignal(false), _cpuStatsUpdateIntervalInSeconds(cpuStatsUpdateIntervalInSeconds),
		_logger(logger){};
	virtual ~CPUUsageThread();

	void start();
	void stop();
	bool isRunning() const;

	uint16_t getCPUUsage() const;

private:
	std::thread _thread;
	std::atomic<bool> _running;
	std::atomic<bool> _stopSignal;
	std::string _networkInterfaceToMonitor;
	int16_t _cpuStatsUpdateIntervalInSeconds;
	std::shared_ptr<spdlog::logger> _logger;

	std::atomic<uint16_t> _cpuUsage;

	void run();

	virtual void newCPUUsageAvailable(uint16_t& cpuUsage) const;
};

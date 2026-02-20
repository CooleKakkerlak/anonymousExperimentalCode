#include "dataReader.h"
#include <iostream>
#include <fstream>

namespace DataReader
{
	std::string dataFolder = "C:\\Users\\6224474\\OneDrive - Universiteit Utrecht\\Documents\\code\\implementing_chromatic_knn\\implementation\\data\\";
	//std::string dataFolder = "/home/ggiezeman/data/datasets/";

	std::vector<std::string> split(std::string str, char delimiter)
	{
		std::vector<std::string> substrings;
		std::string token;
		std::istringstream tokenStream(str);

		while (getline(tokenStream, token, delimiter))
		{
			substrings.push_back(token);
		}

		return substrings;
	}

	Dataset2D readOpenML(Scenario &scenario, MyRandom &random)
	{
		Dataset2D res;
		std::map<std::string, Color_> colorMapping;

		std::ifstream file(dataFolder + scenario.filename);
		std::string text;

		// skip the first few lines
		while (getline(file, text))
		{
			if (text == "@data" || text == "@DATA")
				break;
		}

		while (getline(file, text))
		{
			std::vector<std::string> point = split(text, ',');

			res.points.push_back(Point_2(stod(point[scenario.d0]), stod(point[scenario.d1])));
			if (colorMapping.find(point[point.size() - 1]) == colorMapping.end())
				colorMapping[point[point.size() - 1]] = res.numColors++;
			res.colors.push_back((Color_)colorMapping[point[point.size() - 1]]);
		}

		file.close();

		return res;
	}

	Dataset2D readPointsFile(Scenario &scenario, MyRandom &random)
	{
		Dataset2D res;
		std::set<Color_> colors;

		std::ifstream file(dataFolder + scenario.filename);
		std::string text;
		while (std::getline(file, text))
		{
			std::vector<std::string> args = split(text, ';');

			res.points.push_back(Point_2(stod(args[0]), stod(args[1])));
			res.colors.push_back((Color_)stoi(args[args.size() - 1]));
			colors.insert((Color_)stoi(args[args.size() - 1]));
		}

		file.close();
		res.numColors = colors.size();
		return res;
	}
};
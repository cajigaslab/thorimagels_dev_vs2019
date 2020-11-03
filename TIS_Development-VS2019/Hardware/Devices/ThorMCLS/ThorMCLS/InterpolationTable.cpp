#include "stdafx.h"
#include "InterpolationTable.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>


InterpolationTable::InterpolationTable(std::wstring filePath, EdgeBehavior behavior)
{

	//=== Setup Table Values ===
	loadFromFile(filePath);


	//=== Setup Table Information ===
	setEdgeBehavior(behavior);
}

InterpolationTable::InterpolationTable(EdgeBehavior behavior)
{

	//=== Setup Table Information ===
	setEdgeBehavior(behavior);
}


InterpolationTable::~InterpolationTable(void)
{



}

void InterpolationTable::calculateStats()
{
	//Clear all vectors
	maxDependent.clear();
	minDependent.clear();
	rangeDependent.clear();

	
	//Independent vars
	maxIndependent = std::max_element(table.begin(), table.end(), [&](TableRow& a, TableRow& b) { return (a.independent < b.independent);} )->independent;
	minIndependent = std::max_element(table.begin(), table.end(), [&](TableRow& a, TableRow& b) { return (a.independent > b.independent);} )->independent;
	rangeIndependent = maxIndependent - minIndependent;


	//Dependent vars
	for(int d=0; d<numDependentVars(); d++)
	{
		maxDependent.push_back(std::max_element(table.begin(), table.end(), [&](TableRow& a, TableRow& b) { return (a.dependent[d] < b.dependent[d]);} )->dependent[d]);
		minDependent.push_back(std::max_element(table.begin(), table.end(), [&](TableRow& a, TableRow& b) { return (a.dependent[d] > b.dependent[d]);} )->dependent[d]);
		rangeDependent.push_back(maxDependent[d] - minDependent[d]);
	}

}



void InterpolationTable::loadFromFile(std::wstring filePath)
{

	//After Call Will Need To Recheck For Validity
	tableValidNeedsCalcFlag = true;

	//Clear Old Table Data
	table.clear();


	//Load in file
	std::ifstream inFile;
	inFile.open(filePath);


	if (inFile.is_open())
	{

		//Read Each Line 
		std::string line;
		while (std::getline(inFile, line))
		{

			if(*line.begin() != '#') //Isn't Comment Line
			{
				TableRow row;
				if(!parseLine(line, row))
				{
					break;
				}
				table.push_back(row);
			}
		}
    }


	//If the data is valid, process it for later
	if(isValid())
	{
		std::stable_sort(table.rbegin(), table.rend(), [&](const TableRow& a, const TableRow& b) { return (a.independent > b.independent);});
		calculateStats();
	}

}

bool InterpolationTable::parseLine(const std::string& line, InterpolationTable::TableRow& parsedRowReturn)
{

	std::istringstream iss(line);

	if (iss >> parsedRowReturn.independent)
	{
		double dependent;
		while(iss >> dependent)
		{
			parsedRowReturn.dependent.push_back(dependent);
		}
	}
	else
	{
		return false;
	}

	return parsedRowReturn.dependent.size() > 0;

}



bool InterpolationTable::isValid()
{

	if(tableValidNeedsCalcFlag)
	{

		tableValid = true;

		if(table.size() < 1)
			tableValid = false;

		if(!checkForUniformRowSize())
			tableValid = false;

		tableValidNeedsCalcFlag = false;

	}


	return tableValid;

}


bool InterpolationTable::checkForUniformRowSize()
{

	if(table.size() < 1)
		return false;
	int firstRowSize = static_cast<int>(table[0].dependent.size());

	for(TableRow row: table)
	{
		if(row.dependent.size() != firstRowSize)
			return false;
	}
	return true;

}


double InterpolationTable::interpolateValue(double value, TableRow prev, TableRow next, int dependentVarIndex)
{

	//=== Catch Naive Cases ===
	if(prev.independent == next.independent)
	{
		return prev.dependent[dependentVarIndex];
	}


	double indDiff = next.independent - prev.independent;
	double depDiff = next.dependent[dependentVarIndex] - prev.dependent[dependentVarIndex];

	double valPct = (value - prev.independent) / indDiff;
	double interpolatedValue = valPct * depDiff + prev.dependent[dependentVarIndex];

	return interpolatedValue;

}



double InterpolationTable::interpolate(double independentVariable, int dependentVarIndex)
{
	std::vector<TableRow>::reverse_iterator next, prev;


	//==== Special Cases On Edge Of Tale ===
	BoundsStatus boundsCase = contains(independentVariable);
	if(boundsCase == GREATER_THAN_MAX || boundsCase == EQUAL_TO_MAX)
	{
		next = table.rbegin();
		prev = (edgeBehavior==EXTRAPOLATE ? next+1 : next);
	}
	else if(boundsCase == LESS_THAN_MIN || boundsCase == EQUAL_TO_MIN)
	{
		next = table.rend()-1;
		prev = (edgeBehavior==EXTRAPOLATE ? next-1 : next);
	}


	//=== Find Table Row Before And After ===
	else
	{
		TableRow temp;
		temp.independent = independentVariable;
		next = std::lower_bound(table.rbegin(), table.rend(), temp, [&](const TableRow& a, const TableRow& b) { return (a.independent > b.independent);} );
		prev = next - 1;
	}


	//=== Interpolate Between Surrounding Points ===
	return interpolateValue(independentVariable, *prev, *next, dependentVarIndex);

}


void InterpolationTable::setEdgeBehavior(EdgeBehavior behavior)
{
	edgeBehavior = behavior;	
}
InterpolationTable::EdgeBehavior InterpolationTable::getEdgeBehavior()
{
	return edgeBehavior;
}


std::string InterpolationTable::tableToString()
{

	//Check for valid table
	if(!isValid())
		return "INVALID TABLE";


	//Create Table String
	std::stringstream tableString;

	for(TableRow row : table)
	{
		tableString <<  std::setprecision(2) << std::fixed << std::right  << std::setw(6) << row.independent << "     ";
		for(double dependent : row.dependent)
		{
			tableString << std::setprecision(3) << std::setw(7) << std::left << dependent << "     ";
		}
		tableString << std::endl;
	}

	return tableString.str();
	
}


InterpolationTable::BoundsStatus InterpolationTable::contains(double independentVariable)
{
	if(independentVariable > maxIndependent)
	{
		return GREATER_THAN_MAX;
	}
	else if(independentVariable < minIndependent)
	{
		return LESS_THAN_MIN;
	}
	else if(independentVariable == maxIndependent)
	{
		return EQUAL_TO_MAX;
	}
	else if(independentVariable == minIndependent)
	{
		return EQUAL_TO_MIN;
	}
	else
	{
		return WITHIN_BOUNDS;
	}

}

int InterpolationTable::numDependentVars()
{
	if(isValid())
	{
		return static_cast<int>(table[0].dependent.size());
	}
	else
	{
		return 0;
	}
}


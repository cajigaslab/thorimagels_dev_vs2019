#pragma once

#include <vector>

/// <summary>
/// Interpolation table class representing a table with one independent variable and any number of dependent variables.
/// contains functions to get values of dependent variables corresponding with input independent variables, interpolated
/// between data table points </summary>
/// <remarks>
/// The interpolation table class is designed to load it's table from a file, formated with one variable per column, columns
/// separated by spaces or tabs, and the first column representing the independent variable. The table does not have to be sorted,
/// but discontinuities (meaning two data points with the same independent variable) might not be interpolated properly if not sorted.
/// The table will check it's validity, meaning its ability to return a meaningful value for every variable. Additionally, behavior
/// of interpolations outside of the table boundaries can be changed between extrapolation and returning the bounds.
/// </remarks>
class InterpolationTable
{
public:

	/// <summary> Defines how the table behaves when asked to interpolate a point outside the
	/// range of the independent variable </summary>
	enum EdgeBehavior {
		RETURN_BOUNDS, //Return the last datapoint in range
		EXTRAPOLATE //Extrapolate using the slope of the two data points nearest to the edge
	};




	//=====================================
	//   Setting Up Table
	//=====================================

	/// <summary> Creates a new InterpolationTable loaded from a file </summary>
	/// <param name="filePath"> Path to table file </param>
	/// <param name="behavior"> The edge behavior of this table </param>
	InterpolationTable(std::wstring filePath, EdgeBehavior behavior = EXTRAPOLATE);


	/// <summary> Creates a new empty InterpolationTable </summary>
	/// <param name="behavior"> The edge behavior of this table </param>
	InterpolationTable(EdgeBehavior behavior = EXTRAPOLATE);


	~InterpolationTable();


	/// <summary> Loads data from a file. Use the isValid() function afterwards
	///            to check that the table loaded properly </summary>
	/// <param name="filePath"> The path to the file to load </param>
	/// <remarks> To be read sucessfully, the file must be formatted properly. In the following example, 
	/// iv represents the indepentent variable, and dv[i] represents a depedent variable and the table is length n
	///
	///  #############################################################################
	///  # Lines starting with '#' are commments and are ignored in parsing
	///  # The Table must be rectangular and at least two row and columns long
	///  #############################################################################
	///     iv    dv1   dv2  dv3  ...  dv11  dv12  dv13  ...  dvn
	///     iv    dv1   dv2  dv3  ...  dv11  dv12  dv13  ...  dvn
	///     iv    dv1   dv2  dv3  ...  dv11  dv12  dv13  ...  dvn
	///     iv    dv1   dv2  dv3  ...  dv11  dv12  dv13  ...  dvn
	///     iv    dv1   dv2  dv3  ...  dv11  dv12  dv13  ...  dvn
	/// ############################## END TABLE #####################################
	///
	///</remarks>
	void loadFromFile(std::wstring filePath);




	//==================================
	// Table Information
	//==================================

	/// <summary> Returns if the table is loaded and formated correctly. Includes checks for minimum table size and 
	///           to make sure the table is rectagular </summary>
	/// <returns> True is the table is formated correctly and will return valid results for all inputs </returns>
	bool isValid();


	/// <summary> Sets the edge behavior of the table, effecting the way this table interpolates
	///           values outside the range of the table </summary>
	/// <param name="behavior"> The edge behavior to use </param>
	void setEdgeBehavior(EdgeBehavior behavior);


	/// <summary> Get the set edge behavior of this table </summary>
	/// <returns> The currently set edge behavior </returns>
	EdgeBehavior getEdgeBehavior();


	/// <summary> Used to get the number of dependent variables in the loaded table. Will return 0 if table is not valid </summary>
	/// <returns> The number of dependent variables in this table </returns>
	int numDependentVars();

	
	/// <summary> Returns a string representation of this table </summary>
	/// <returns> String representation of this table, formatted to be valid to this table's parser  </returns>
	std::string tableToString();




	//==============================
	// Core Functionality
	//==============================

	/// <summary> Return the value of the requested dependent variable corresponding to the input
	///           independent variable, linearily interpolated to estimate between data points </summary>
	/// <param name="independentVariable"> The value of the independent variable </param>
	/// <param name="dependentVarIndex"> Which dependent variable to return </param>
	/// <returns> The interpolated value of the dependent variable </returns>
	virtual double interpolate(double independentVariable, int dependentVarIndex);

	


private:


	//===========================
	//   Table Status
	//===========================
	
	/// <summary> The bounds status enum is used to specify where a value lies within the table.
	/// The reason an enum is being used is to make absolutely clear what the return values of
	///  the contains() function means, as opposed to returning an integer which would be non-obvious
	/// </summary>
	enum BoundsStatus {
		GREATER_THAN_MAX,
		EQUAL_TO_MAX,
		LESS_THAN_MIN,
		EQUAL_TO_MIN,
		WITHIN_BOUNDS
	};
	
	
	/// <summary> Determins if this data point is within the range of independent variables in the table </summary>
	/// <param name="independentVariable"> The value of the independent variable to check </param>
	/// <returns> BoundsStatus enum defining where the value lies within the table range </returns>
	BoundsStatus contains(double independentVariable);




	//===================================
	//  Table Statistics
	//===================================

	/// <summary> Calculates min, max, and range statistics on all variables in the table </summary>
	void calculateStats();

	std::vector<double> minDependent, maxDependent, rangeDependent;
	double minIndependent, maxIndependent, rangeIndependent;




	//==================================
	// Table Structure
	//==================================

	/// <summary> A single row in the table. Each row contains an independent variable, and a
	/// set of dependent variables of any size </summary>
	struct TableRow {
		double independent;
		std::vector<double> dependent;
	};

	std::vector<TableRow> table; //Vector of table rows that make up the table
	EdgeBehavior edgeBehavior; //What this table will do when interpolating values outside table bounds




	//==============================
	// Core Functionality
	//==============================

	/// <summary> Return the interpolated value of a dependent variable based on the input independent variable.
	///           A simple linear interpolation is used that estimates the postition between data points by fitting
	///           a straight line to the two input rows
	/// </summary>
	/// <param name="value"> The value of the independent variable </param>
	/// <param name="prev">  TableRow to use for the interpolation </param>
	/// <param name="next">  Second TableRow to use for the interpolation </param>
	/// <param name="dependentVarIndex"> Which dependent var to return </param>
	/// <returns> The interpolated value of the requested dependent variable, based on the two input rows </returns>
	double interpolateValue(double value, TableRow prev, TableRow next, int dependentVarIndex);




	//====================================
	// Table Validity
	//====================================

	/// <summary> Check that the table is rectangular, meaning all rows contain the same number of variables </summary>
	/// <returns> True if the all rows in the table contain the same number of variables, False if not </returns>
	bool checkForUniformRowSize();

	bool tableValidNeedsCalcFlag; //Recalculate table validity on next call to is valid, used to buffer valid status
	bool tableValid; //Store the table validity to only recalculate it when necessary




	//=================================
	// Table IO
	//=================================

	/// <summary> Parse a single table row </summary>
	/// <param name="line"> The row to parse in string form </param>
	/// <param name="parsedRowReturn"> A reference to a TableRow to be filled with the parsed data </param>
	/// <returns> True if the row was succesfully parsed, False if there was an error </returns>
	bool parseLine(const std::string& line, TableRow &parsedRowReturn);

};


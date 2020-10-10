// $Id: ubigint.cpp,v 1.9 2020-07-02 15:38:57-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that){//: ubig_value (that) {
   //DEBUGF ('~', this << " -> " << uvalue)

   // Turn the unsigned long that into individual numbers
   // to be passed into the ubigint.
   int temp = 0;
   // In the case where 0 is given.
   if ( that == 0){
      ubig_value.push_back(0);
   }
   // Otherwise, continue.
   while (that != 0){
      // Get the least order digit %10
      temp = that % 10; // Stores the remainder.
      // Push the remainder onto the vector.
      ubig_value.push_back(temp);
      // Divide the number by 10.
      that = that / 10;
      // Re-loop at the start.
   }
   printf("Not String version debug\n");
   for (int iter = ubig_value.size()-1; iter >= 0; iter-- ){
      printf("%i", ubig_value[iter]);
   }
   printf("\n");
}

ubigint::ubigint (const string& that){//: uvalue(0) {
   //DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      
      //uvalue = uvalue * 10 + digit - '0'; - Given to us.

      ubig_value.insert(ubig_value.begin(), digit - '0');

   }
   printf("String version debug\n");
   for (int iter = ubig_value.size()-1; iter >= 0; iter-- ){
      printf("%i", ubig_value[iter]);
   }
   printf("\n");
}

ubigint ubigint::operator+ (const ubigint& that) const {
   //DEBUGF ('u', *this << "+" << that);
   printf("chicken tasty ubigint\n");
   ubigint result;
   //DEBUGF ('u', result);

   // Initializing variables.
   int carry = 0; // Initialized at 0.
   int smaller_size = 0; // Initialized at 0.
   int larger_size  = 0; // Initialized at 0.
   
   
   bool that_is_larger = false; // Initialized with false.

   // Perform comparisons to find the smaller vector length.
   if ( that.ubig_value.size() > ubig_value.size() ){
      // if that > this, then smaller_size = this.
      smaller_size   = ubig_value.size();
      larger_size    = that.ubig_value.size();
      that_is_larger = true;
      printf("that > this\n");
   } else {
      // Otherwise, this >= that.
      smaller_size   = that.ubig_value.size();
      larger_size    = ubig_value.size();
      that_is_larger = false;
      printf("this >= that\n");
   }
   printf("smaller_size = %i\n", smaller_size);
   printf("larger_size = %i\n", larger_size);
   // In the case where this and that are of equal size,
   // smaller_size and larger_size are equal.

   // Core loop #1.
   // This loop iterates where both vectors have values.
   // It takes the values and adds them, taking into account
   // Of a possible carry value.
   for ( int iter = 0; iter <= smaller_size-1; iter++ ){
      // Initliaze the temporary result.
      int t_result = 0;
      // Perform the addition, with the carry.
      t_result = that.ubig_value[iter] + ubig_value[iter] + carry;
      // Calculate the carry value.
      carry = t_result / 10; // If it is > 9, carry = 1.
      // Shave off the remainder, if a remainder.
      t_result = t_result % 10;
      // Push the t_result into the resulting bigint.
      result.ubig_value.push_back(t_result);
      printf("Core Loop #1 progress\n");
      printf("carry value    = %i\n", carry);
      printf("t_result value = %i\n", t_result);
   }
   // Core loop #2.
   // This loop iterates over the larger vector,
   // adding the rest of the values onto the result
   // bigint vector.
   // This loop does not perform if the sizes are equal.
   for ( int iter = smaller_size-1; iter < larger_size-1; iter++ ){
      // Initialize the temporary result.
      int t_result = 0;
      // Perform the addition with the carry.
      // The addition depends on the larger vector.
      if ( that_is_larger == true ){
         // Iterate over "that" vector.
         t_result = that.ubig_value[iter] + carry;
         // Calculate the carry value.
         carry = t_result / 10; // If it is > 9, carry = 1.
         // Shave off the remainder, if a remainder.
         t_result = t_result % 10;
         // Push the t_result into the resulting bigint.
         result.ubig_value.push_back(t_result);
         printf("Core Loop #2 progress\n");
         printf("carry value    = %i\n", carry);
         printf("t_result value = %i\n", t_result);
         printf("that_is_larger == true\n");
      } else {
         // Otherwise, that_is_larger == false.
         // We iterate over "this" vector.
         t_result = ubig_value[iter] + carry;
         // Calculate the carry value.
         carry = t_result / 10; // IF it is > 9, carry = 1.
         // Shave off the remainder, if a remainder.
         t_result = t_result % 10;
         // Push the t_result into the resulting bigint.
         result.ubig_value.push_back(t_result);
         printf("Core Loop #2 progress\n");
         printf("carry value    = %i\n", carry);
         printf("t_result value = %i\n", t_result);
         printf("that_is_larger == false\n");
      }
   }
   // Final operations.
   // Check if the carry value >= 1.
   // If it is, add the carry onto the result bigint vector.
   printf("Final operations\n");
   if ( carry >= 1 ){
      printf("adding on the carry");
      result.ubig_value.push_back(carry);
   }
   // Otherwise, do nothing.
   printf("carry value    = %i\n", carry);

   // Return the resulting ubigint.
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   return ubigint (uvalue - that.uvalue);
}

ubigint ubigint::operator* (const ubigint& that) const {
   return ubigint (uvalue * that.uvalue);
}

void ubigint::multiply_by_2() {
   uvalue *= 2;
}

void ubigint::divide_by_2() {
   uvalue /= 2;
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   return uvalue == that.uvalue;
}

bool ubigint::operator< (const ubigint& that) const {
   return uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const ubigint& that) {
   // Reverse iterate over that.ubig_value.
   for (int iter = that.ubig_value.size()-1; iter >= 0; iter--){
      // Print out the information at index iter.
      out << static_cast<char>(that.ubig_value[iter]+'0');
   }
   //return out << "ubigint(" << that.uvalue << ")";
   return out;
}


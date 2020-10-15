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

ubigint::ubigint (unsigned long that){
   DEBUGF ('~', this << " -> " << uvalue)

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
   // Trim the leading zeros in front of the first value.
   if ( ubig_value.size() > 1 && ubig_value.back() == 0 ){
      // If the vector is only one element, this will not run.
      // Initialize a temporary integer.
      int trim_temp = 0;
      // Set it to the ubig_value.back();
      trim_temp = ubig_value.back();
      // Iterate through and pop_back() as long as temp == 0.
      while ( trim_temp == 0 && ubig_value.size() > 1 ){
         // Remove the 0 at the back.
         ubig_value.pop_back();
         // Set temp to the new back.
         trim_temp = ubig_value.back();
         // Go back to the start.
      }      
   } else if ( ubig_value.size() < 1 ){
      // If the vector is empty, make it 0.
      ubig_value.push_back(0);
   }
}

ubigint::ubigint (const string& that){
   DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }

      ubig_value.insert(ubig_value.begin(), digit - '0');

   }
   // Trim the leading zeros in front of the first value.
   if ( ubig_value.size() > 1 && ubig_value.back() == 0 ){
      // If the vector is only one element, this will not run.
      // Initialize a temporary integer.
      int trim_temp = 0;
      // Set it to the ubig_value.back();
      trim_temp = ubig_value.back();
      // Iterate through and pop_back() as long as temp == 0.
      while ( trim_temp == 0 && ubig_value.size() > 1 ){
         // Remove the 0 at the back.
         ubig_value.pop_back();
         // Set temp to the new back.
         trim_temp = ubig_value.back();
         // Go back to the start.
      }      
   } else if ( ubig_value.size() < 1 ){
      // If the vector is empty, make it 0.
      ubig_value.push_back(0);
   }
}

ubigint ubigint::operator+ (const ubigint& that) const {
   DEBUGF ('u', *this << "+" << that);
   ubigint result;
   DEBUGF ('u', result);

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
   } else {
      // Otherwise, this >= that.
      smaller_size   = that.ubig_value.size();
      larger_size    = ubig_value.size();
      that_is_larger = false;
   }
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
   }
   // Core loop #2.
   // This loop iterates over the larger vector,
   // adding the rest of the values onto the result
   // bigint vector.
   // This loop does not perform if the sizes are equal.
   for ( int iter = smaller_size; iter < larger_size; iter++ ){
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
      }
   }
   // Final operations.
   // Check if the carry value >= 1.
   // If it is, add the carry onto the result bigint vector.
   if ( carry >= 1 ){
      result.ubig_value.push_back(carry);
   }
   // Otherwise, do nothing.

   // Return the resulting ubigint.
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   // Step 1. Sort this and that into larger and smaller
   //    booleans that_is_bigger, and this_is_bigger.
   //    If they are completely equal, return 0.
   // Step 2. Subtract the values. Always smaller from bigger.

   // Initialize the resulting ubigint.
   ubigint result;
   // Initialize the variables
   bool this_is_bigger = false;
   bool that_is_bigger = false;

   // Core Loop #1.
   // Sorting this and that.

   // Check size differences.
   if ( that.ubig_value.size() > ubig_value.size() ){
      // If that > this, then that_is_bigger = true.
      that_is_bigger = true;
   }
   else if ( ubig_value.size() > that.ubig_value.size() ){
      // else, if this > that, then this_is_bigger = true
      this_is_bigger = true;
   }
   else {
      // Otherwise, they are the same size, iterate
      // through to find the larger int

      // Initialize required variables
      int size        = that.ubig_value.size()-1;
      int break_check = 0;

      // Iterate with a while loop.
      while ( size > -1 && break_check == 0 ){
         // Compare the two numbers at position l_s
         if ( ubig_value[size] > that.ubig_value[size] ){
            // if this[s] > that[s]
            // then this is bigger.
            this_is_bigger = true;
            // Increment break check.
            break_check = break_check + 1;
         }
         else if ( that.ubig_value[size] > ubig_value[size] ){
            // if that[s] > this[s]
            // then that is bigger.
            that_is_bigger = true;
            // Increment break check.
            break_check = break_check + 1;
         }
         else {
            // Otherwise, the two numbers are equal.
            // First, de-increment the size.
            size = size - 1;
            // Next, check if that was the lowest order number.
            if ( size <= -1 ){
               // They were the exact same number,
               // Return 0.
               result.ubig_value.push_back(0);
               return result;
            }
         }

      }
   }

   // Core Loop #2
   // Subtract the different numbers.

   // Initialize the required variables.
   int iter      = 0; // iterator value.
   int carry     = 0; // carry value.
   int this_size = ubig_value.size(); // size of this
   int that_size = that.ubig_value.size(); // size of that

   // Subtracting can be done in two different while loops,
   // depending on whether this or that is bigger than the other.

   // In the case where this > that
   while ( this_is_bigger && iter <= that_size ){
      // while this > that, and iter does < that_size
      // Initialize a temporary result.
      int t_result = 0;
      // Perform the subtraction with the carry variable.
      t_result = ubig_value[iter] - that.ubig_value[iter] + carry;
      // Reset the carry value to 0.
      carry = 0;
      // Check if the result < 0
      if ( t_result < 0 ){
         // set the carry to -1.
         carry = -1;
         // Add 10 to the result.
         t_result = t_result + 10;
      }
      // Push the t_result onto the result vector.
      result.ubig_value.push_back(t_result);
      // Increment the iter variable.
      iter = iter + 1;
      // Loop back to the top.
   }

   // In the case where that > this
   while ( that_is_bigger && iter <= this_size ){
      // while that > this, and iter < this_size
      // Initialize a temporary result.
      int t_result = 0;
      // Perform the subtraction with the carry variable.
      t_result = that.ubig_value[iter] - ubig_value[iter] + carry;
      // Reset the carry to 0.
      carry = 0;
      // Check if the result < 0
      if ( t_result < 0 ){
         // set the carry to -1.
         carry = -1;
         // Add 10 to the result.
         t_result = t_result + 10;
      }
      // Push the t_result onto the result vector.
      result.ubig_value.push_back(t_result);
      // Increment the iter variable.
      iter = iter + 1;
      // Loop back to the top.
   }

   // Trim the leading zeros in front of the first value.
   if ( result.ubig_value.size() > 1 && result.ubig_value.back() == 0 ){
      // If the vector is only one element, this will not run.
      // Initialize a temporary integer.
      int trim_temp = 0;
      // Set it to the ubig_value.back();
      trim_temp = result.ubig_value.back();
      // Iterate through and pop_back() as long as temp == 0.
      while ( trim_temp == 0 && result.ubig_value.size() > 1 ){
         // Remove the 0 at the back.
         result.ubig_value.pop_back();
         // Set temp to the new back.
         trim_temp = result.ubig_value.back();
         // Go back to the start.
      }      
   }

   // Return the result vector
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   // Initialize required variables.
   int order     = 0;  // To keep track of the highest order.
   int carry     = 0;  // Carry variable.
   int this_size = ubig_value.size(); // this.size()
   int that_size = that.ubig_value.size(); // that.size()
   ubigint result; // resulting ubigint.

   // While loop #1.
   while ( order < that_size ){
      // While the order < that_size.

      // Initialize required variables.
      int this_iter = 0; // iterator for this
      ubigint t_vect;    // temporary vector

      // Push back any 0s needed onto the t_vect.
      for ( int iter = 0; iter < order; iter++ ){
         // Push a 0 onto the t_vect.
         t_vect.ubig_value.push_back(0);
      }

      // While loop #2
      while ( this_iter != this_size ){
         // while this_iter != this_size

         // Initialize required variables
         int t_result = 0; // temporary result
         
         // Perform the multiplication, adding on a carry
         t_result = ubig_value[this_iter] * that.ubig_value[order]
         + carry;
         // Calculate the carry.
         carry = t_result / 10;
         // Set t_result to the remainder.
         t_result = t_result % 10;
         // Push t_result onto the temporary vector.
         t_vect.ubig_value.push_back(t_result);
         // Iterate this_iter by 1.
         this_iter = this_iter + 1;
         
         // Loop back to the top
      }

      // Outside of While loop #2, add the t_vect onto the result.
      result = result + t_vect;
      // Iterate the order by 1.
      order = order + 1;

      // Loop back to the top.
   }
   // Final operations.
   // Check if the carry value >= 1.
   // If it is, add the carry onto the result bigint vector.
   if ( carry >= 1 ){
      result.ubig_value.push_back(carry);
   }
   // Otherwise, do nothing.
   
   return result;
}

void ubigint::multiply_by_2() {
   // Initialize required variables
   int iter  = 0;
   int carry = 0;
   int size  = ubig_value.size();

   // Core while loop.
   while ( iter < size ){
      // Initialize the required variable
      int t_result = 0;
      // Perform this*2 by adding it to itself with the carry.
      t_result = ubig_value[iter] + ubig_value[iter] + carry;
      // Calculate the carry
      carry = t_result / 10;
      // Put the remainder into t_result.
      t_result = t_result % 10;
      // Change the data in this[iter] to t_result.
      ubig_value[iter] = t_result;
      // Increment the iterator.
      iter = iter + 1;
   }

   // Final adjustments, check if carry > 0
   if ( carry > 0 ){
      // Push the carry onto this
      ubig_value.push_back(carry);
   }

   // Program over
}

void ubigint::divide_by_2() {
   // Initialize required variables.
   int iter  = 0; // Iterator variable
   int size  = ubig_value.size(); // size variable.

   // Core while loop
   while ( iter < size ){
      // while iter < this.size
      // Divide the current number by 2.
      ubig_value[iter] = ubig_value[iter] / 2;
      // Check if the current value is the MSB.
      if ( iter == size-1 ){
         // Do nothing
      }
      else {
         // Otherwise, check if the next value is odd.
         if ( ubig_value[iter+1] % 2 == 1 ){
            // Add 5 onto the current value.
            ubig_value[iter] = ubig_value[iter] + 5;
         }
         // Otherwise, do nothing.
      }

      // Increment iter by 1.
      iter = iter + 1;
   }

   // Trim the leading zeros in front of the first value.
   if ( ubig_value.size() > 1 && ubig_value.back() == 0 ){
      // If the vector is only one element, this will not run.
      // Initialize a temporary integer.
      int trim_temp = 0;
      // Set it to the ubig_value.back();
      trim_temp = ubig_value.back();
      // Iterate through and pop_back() as long as temp == 0.
      while ( trim_temp == 0 && ubig_value.size() > 1 ){
         // Remove the 0 at the back.
         ubig_value.pop_back();
         // Set temp to the new back.
         trim_temp = ubig_value.back();
         // Go back to the start.
      }      
   } else if ( ubig_value.size() < 1 ){
      // If the vector is empty, make it 0.
      ubig_value.push_back(0);
   }

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
   // Similar to operator<, we can find out
   // if this == that.

   // Initialize required variables.
   // None needed.

   // Compare the sizes of the vectors.
   if ( ubig_value.size() > that.ubig_value.size() ){
      // if this > that, this is bigger. return false.
      return false;
   }
   else if ( that.ubig_value.size() > ubig_value.size() ){
      // if that > this, that is bigger. return false.
      return false;
   }
   else {
      // They are of the same size, compare each by
      // highest order, descending.

      // Initializing required variables.
      int size        = that.ubig_value.size()-1;
      int break_check = 0;
      // Iterate using a while loop.
      while ( size > -1 && break_check == 0 ){
         // Compare the two numbers.
         if ( ubig_value[size] > that.ubig_value[size] ){
            // this > that, return false.
            return false;
         }
         else if ( that.ubig_value[size] > ubig_value[size] ){
            // that > this, return false.
            return false;
         }
         else {
            // They are the same value, continue iterating.
            // De-increment size.
            size = size - 1;
            // Check if the lowest order has been reached.
            if ( size <= -1 ){
               // If this is the case, this == that.
               return true;
            }
            // Otherwise, continue from the top.
         }
      }
   }
   
   // If the program somehow gets here, return false.
   return false;
}

bool ubigint::operator< (const ubigint& that) const {
   // Similar to Step 1 of operator-, we can
   // find out which is bigger, this or that.

   // Initialize the required variable.
   bool this_is_bigger = false;
   bool that_is_bigger = false;
   
   // Compare the sizes of the vectors.
   if ( ubig_value.size() > that.ubig_value.size() ){
      // If this > that, this is bigger.
      this_is_bigger = true;
   }
   else if ( that.ubig_value.size() > ubig_value.size() ){
      // If that > this, that is bigger.
      that_is_bigger = true;
   }
   else {
      // In this case they are of equal sizes,
      // compare each by highest order, descending.

      // Initialize required variables.
      int size        = that.ubig_value.size()-1;
      int break_check = 0;
      // Iterate using a while loop.
      while ( size > -1 && break_check == 0 ){
         // Compare the two numbers.
         if ( ubig_value[size] > that.ubig_value[size] ){
            // if this[s] > that[s], this is bigger.
            this_is_bigger = true;
            // Incrememnt the break checker.
            break_check = break_check + 1;
         }
         else if ( that.ubig_value[size] > ubig_value[size] ){
            // if that[s] > this[s], that is bigger.
            that_is_bigger = true;
            // Increment the break checker.
            break_check = break_check + 1;
         }
         else {
            // Otherwise, they are exactly equal.
            // De-increment size.
            size = size - 1;
            // Check if the lowest order has been reached.
            if ( size <= -1 ){
               // this == that, return false.
               return false;
            }
            // Otherwise, continue from the top.
         }
      }
   }

   // Once comparisons are finished, compare the results.
   // operator< is this < that, check this case.
   if ( that_is_bigger && this_is_bigger == false ){
      // this < that
      return true;
   }
   // Otherwise, return false.
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) {
   // Reverse iterate over that.ubig_value.
   for (int iter = that.ubig_value.size()-1; iter >= 0; iter--){
      // Print out the information at index iter.
      out << static_cast<char>(that.ubig_value[iter]+'0');
   }
   return out;
}


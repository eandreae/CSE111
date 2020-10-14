// $Id: bigint.cpp,v 1.2 2020-01-06 13:39:55-08 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   // Initialize bigint result.
   bigint result;

   // Four cases for addition.
   // a + b,   add (a+b)
   // -a + b,  sub (b-a)
   // -a + -b, add -(a+b)
   // a + -b,  sub (a-b)

   // Case 1: a + b
   if ( is_negative == false && that.is_negative == false ){
      // If both a and b are positive, add like normal.
      // a + b
      result.uvalue = uvalue + that.uvalue;
      // Set the sign.
      result.is_negative = false;
   }
   // Case 2: b - a
   else if ( is_negative == true && that.is_negative == false ){
      // If a is negative, and b is positive, subtract b from a.
      // b - a
      // Subtract the larger value from the smaller value.
      // Check if a > b.
      if ( uvalue > that.uvalue ){
         // Subtract b from a, sign is negative.
         // -(a - b)
         result.uvalue = uvalue - that.uvalue;
         // Set the sign to negative.
         result.is_negative = true;
      }
      else {
         // Otherwise, a <= b. Subtract normally.
         // b - a
         result.uvalue = that.uvalue - uvalue;
         // Set the sign to positive.
         result.is_negative = false;
      }
   }
   // Case 3: -(a + b)
   else if ( is_negative == true && that.is_negative == true ){
      // If a and b are negative, add and make sign negative.
      // -(a + b)
      result.uvalue = uvalue + that.uvalue;
      // Set the sign to negative.
      result.is_negative = true;
   }
   // Case 4: a - b
   else if ( is_negative == false && that.is_negative == true ){
      // If a is positive, and b is negative, subtract a from b.
      // a - b
      // Check if b > a
      if ( that.uvalue > uvalue ){
         // Perform b - a instead, making the sign negative.
         result.uvalue = that.uvalue - uvalue;
         // Set the sign to negative
         result.is_negative = true;
      }
      else {
         // Otherwise, b <= a. Subtract normally.
         result.uvalue = uvalue - that.uvalue;
         // Set the sign to positive.
         result.is_negative = false;
      }
   }

   if ( uvalue > that.uvalue ){
      cout << uvalue << " > " << that.uvalue << endl;
   }
   else if ( that.uvalue > uvalue ){
      cout << uvalue << " < " << that.uvalue << endl;
   }
   else if ( uvalue == that.uvalue ){
      cout << uvalue << " = " << that.uvalue << endl;
   }

   return result;
   // Adding two ubigints.
}

bigint bigint::operator- (const bigint& that) const {
   // Initialize result bigint.
   bigint result;
   // a - b
   // b - a
   // Run the comparisons to if (this - that) or (that - this).
   // Case 1: a - b
   if ( uvalue > that.uvalue ){
      // if a > b, subtraction is possible.
      result.uvalue = uvalue - that.uvalue;
      // Set the sign to positive.
      result.is_negative = false;
      // Debug information
      cout << uvalue << " > " << that.uvalue << endl;
   }
   else if ( that.uvalue > uvalue ){
      // if b > a, subtract the other way, making the sign negative.
      result.uvalue = that.uvalue - uvalue;
      // Set the sign to negative
      result.is_negative = true;
      // Debug information
      cout << uvalue << " < " << that.uvalue << endl;
   }
   else {
      // if this == that, subtraction is possible.
      result.uvalue = uvalue - that.uvalue;
      // Debug information.
      cout << uvalue << " = " << that.uvalue << endl;
   }
   // Return the result vector.
   return result;
}


bigint bigint::operator* (const bigint& that) const {
   bigint result = uvalue * that.uvalue;
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result = uvalue / that.uvalue;
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result = uvalue % that.uvalue;
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << "bigint(" << (that.is_negative ? "-" : "+")
              << "," << that.uvalue << ")";
}


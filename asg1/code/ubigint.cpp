// $Id: ubigint.cpp,v 1.9 2020-07-02 15:38:57-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that){//: ubig_value (that) {
   //DEBUGF ('~', this << " -> " << uvalue)
   // Turn the unsigned long that into individual numbers
   // to be passed into the ubigint.
   int temp = 0;
   while (that != 0){
      // Get the least order digit %10
      temp = that % 10; // Stores the remainder.
      // Push the remainder onto the vector.
      

      


   }

}

ubigint::ubigint (const string& that): uvalue(0) {
   //DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      uvalue = uvalue * 10 + digit - '0';
      // 4829
      // %10 9 -> v0
      // /10 482
      // %10 2 -> v1
      // /10 48
      // %10 8 -> v2
      // /10 4
      // %10 4 -> v3
      // /10 0
      // 9 2 8 4
   }
}

ubigint ubigint::operator+ (const ubigint& that) const {
   DEBUGF ('u', *this << "+" << that);
   ubigint result (uvalue + that.uvalue);
   DEBUGF ('u', result);
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
   return out << "ubigint(" << that.uvalue << ")";
}


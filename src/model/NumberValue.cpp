//
// Created by deufrai on 01/03/2021.
//

#include "NumberValue.h"

NumberValue::NumberValue(const std::string &name, const std::string &label,
                         const double &value, const std::string &format,
                         const double &min, const double &max, const double &step)
: ValueBase(name, label),
  _value(value),
  _format(format),
  _min(min),
  _max(max),
  _step(step)
{

}
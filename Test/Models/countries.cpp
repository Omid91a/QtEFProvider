#include "countries.h"

Countries::Countries() : DatabaseModel()
{
}


int Countries::id() const
{
   return _Id;
}


void Countries::setId(int id)
{
   _Id = id;
}


QString Countries::name() const
{
   return _Name;
}


void Countries::setName(const QString& name)
{
   _Name = name;
}


QString Countries::display() const
{
   return _Display;
}


void Countries::setDisplay(const QString& display)
{
   _Display = display;
}

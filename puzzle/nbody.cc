/*
 * The Great Computer Language Shootout
 * http://shootout.alioth.debian.org/
 *
 * C version contributed by Christoph Bauer
 * converted to C++ and modified by Paul Kitchin
 */

#include <cmath>
#include <stdio.h>
#include <stdlib.h>

struct Vector3
{
  Vector3(double x, double y, double z)
    : x(x), y(y), z(z)
  {
  }

  double x;
  double y;
  double z;
};

struct Planet
{
  Planet(const Vector3& position, const Vector3& velocity, double mass)
    : position(position), velocity(velocity), mass(mass)
  {
  }

  Vector3 position;
  Vector3 velocity;
  const double mass;
};

inline Vector3& operator-=(Vector3& lhs, const Vector3& rhs)
{
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  lhs.z -= rhs.z;
  return lhs;
}

inline Vector3 operator-(Vector3 lhs, const Vector3& rhs)
{
  return (lhs -= rhs);
}

inline Vector3 operator-(Vector3 lhs)
{
  return Vector3(-lhs.x, -lhs.y, -lhs.z);
}

inline Vector3& operator+=(Vector3& lhs, const Vector3& rhs)
{
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  lhs.z += rhs.z;
  return lhs;
}

inline Vector3 operator+(Vector3 lhs, const Vector3& rhs)
{
  return (lhs += rhs);
}

inline Vector3& operator*=(Vector3& lhs, double rhs)
{
  lhs.x *= rhs;
  lhs.y *= rhs;
  lhs.z *= rhs;
  return lhs;
}

inline Vector3 operator*(Vector3 lhs, double rhs)
{
  return (lhs *= rhs);
}

inline Vector3 operator*(double lhs, Vector3 rhs)
{
  return (rhs *= lhs);
}

inline Vector3& operator/=(Vector3& lhs, double rhs)
{
  lhs.x /= rhs;
  lhs.y /= rhs;
  lhs.z /= rhs;
  return lhs;
}

inline Vector3 operator/(Vector3 lhs, double rhs)
{
  return (lhs /= rhs);
}

inline double magnitude_squared(const Vector3& vec)
{
  return ((vec.x * vec.x) + (vec.y * vec.y)) + (vec.z * vec.z);
}

inline double magnitude(const Vector3& vec)
{
  return std::sqrt(magnitude_squared(vec));
}

void advance(int nbodies, Planet* bodies, double delta_time)
{
  for (Planet* p1 = bodies; p1 != bodies + nbodies; ++p1)
  {
    for (Planet* p2 = p1 + 1; p2 != bodies + nbodies; ++p2)
    {
      Vector3 difference = p1->position - p2->position;
      double distance_squared = magnitude_squared(difference);
      double distance = std::sqrt(distance_squared);
      double magnitude = delta_time / (distance * distance_squared);
      p1->velocity -= difference * p2->mass * magnitude;
      p2->velocity += difference * p1->mass * magnitude;
    }
  }
  for (Planet* p = bodies; p != bodies + nbodies; ++p)
  {
    p->position += delta_time * p->velocity;
  }
}

void advance2(int nbodies, Planet* bodies, double delta_time)
{
  for (Planet* p1 = bodies; p1 != bodies + nbodies; ++p1)
  {
    for (Planet* p2 = p1 + 1; p2 != bodies + nbodies; ++p2)
    {
      Vector3 difference = p1->position - p2->position;
      double distance_squared = magnitude_squared(difference);
      double distance = std::sqrt(distance_squared);
      double magnitude = delta_time / (distance * distance_squared);
      double planet2_mass_magnitude = p2->mass * magnitude;
      double planet1_mass_magnitude = p1->mass * magnitude;
      p1->velocity -= difference * planet2_mass_magnitude;
      p2->velocity += difference * planet1_mass_magnitude;
    }
  }
  for (Planet* p = bodies; p != bodies + nbodies; ++p)
  {
    p->position += delta_time * p->velocity;
  }
}

unsigned int const number_of_bodies = 5;
double const days_per_year = 365.24;
double const solar_mass = 4 * M_PI * M_PI;

Planet bodies[5] =
{
   Planet(Vector3(0, 0, 0), Vector3(0, 0, 0), solar_mass),
   Planet(Vector3(4.84143144246472090e+00,
         -1.16032004402742839e+00,
         -1.03622044471123109e-01),
         Vector3(1.66007664274403694e-03 * days_per_year,
         7.69901118419740425e-03 * days_per_year,
         -6.90460016972063023e-05 * days_per_year),
         9.54791938424326609e-04 * solar_mass),
   Planet(Vector3(8.34336671824457987e+00,
         4.12479856412430479e+00,
         -4.03523417114321381e-01),
         Vector3(-2.76742510726862411e-03 * days_per_year,
         4.99852801234917238e-03 * days_per_year,
         2.30417297573763929e-05 * days_per_year),
         2.85885980666130812e-04 * solar_mass),
   Planet(Vector3(1.28943695621391310e+01,
         -1.51111514016986312e+01,
         -2.23307578892655734e-01),
         Vector3(2.96460137564761618e-03 * days_per_year,
         2.37847173959480950e-03 * days_per_year,
         -2.96589568540237556e-05 * days_per_year),
         4.36624404335156298e-05 * solar_mass),
   Planet(Vector3(1.53796971148509165e+01,
         -2.59193146099879641e+01,
         1.79258772950371181e-01),
         Vector3(2.68067772490389322e-03 * days_per_year,
         1.62824170038242295e-03 * days_per_year,
         -9.51592254519715870e-05 * days_per_year),
         5.15138902046611451e-05 * solar_mass)
};

double energy()
{
  double total_energy = 0.0;
  for (Planet * planet1 = bodies; planet1 != bodies + number_of_bodies; ++planet1)
  {
    total_energy += 0.5 * planet1->mass * magnitude_squared(planet1->velocity);
    for (Planet * planet2 = planet1 + 1; planet2 != bodies + number_of_bodies; ++planet2)
    {
      Vector3 difference = planet1->position - planet2->position;
      double distance = magnitude(difference);
      total_energy -= (planet1->mass * planet2->mass) / distance;
    }
  }
  return total_energy;
}

void offset_momentum()
{
  Vector3 momentum(bodies[1].velocity * bodies[1].mass);
  for (Planet * planet = bodies + 2; planet != bodies + number_of_bodies; ++planet)
  {
    momentum += planet->velocity * planet->mass;
  }
  bodies[0].velocity = -momentum / solar_mass;
}

int main(int argc, char * * argv)
{
  int n = atoi(argv[1]);

  offset_momentum();

  printf ("%.9f\n", energy());

  for (int i = 1; i <= n; ++i)
  {
    advance(number_of_bodies, bodies, 0.01);
  }

  printf ("%.9f\n", energy());
}

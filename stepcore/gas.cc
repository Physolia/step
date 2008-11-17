/* This file is part of StepCore library.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   StepCore library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepCore library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepCore; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "gas.h"
#include "types.h"
#include "constants.h"
#include <cstdlib>
#include <QtGlobal>

namespace StepCore
{

STEPCORE_META_OBJECT(GasParticle, QT_TR_NOOP("Gas particle"), 0, STEPCORE_SUPER_CLASS(Particle),)

STEPCORE_META_OBJECT(GasLJForce, QT_TR_NOOP("Lennard-Jones force"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, depth, QT_TR_NOOP("J"), QT_TR_NOOP("Potential depth"), depth, setDepth)
    STEPCORE_PROPERTY_RW(double, rmin, QT_TR_NOOP("m"), QT_TR_NOOP("Distance at which the force is zero"), rmin, setRmin)
    STEPCORE_PROPERTY_RW(double, cutoff, QT_TR_NOOP("m"), QT_TR_NOOP("Cut-off distance"), cutoff, setCutoff))

STEPCORE_META_OBJECT(GasLJForceErrors, QT_TR_NOOP("Errors class for GasLJForce"), 0,
    STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_RW(double, depthVariance, QT_TR_NOOP("J"),
            QT_TR_NOOP("Potential depth variance"), depthVariance, setDepthVariance)
    STEPCORE_PROPERTY_RW(double, rminVariance, QT_TR_NOOP("m"),
            QT_TR_NOOP("Variance of the distance at which the force is zero"), rminVariance, setRminVariance))

// XXX: Check units for 2d
// XXX: add cmPosition and cmVelocity
STEPCORE_META_OBJECT(Gas, QT_TR_NOOP("Particle gas"), 0, STEPCORE_SUPER_CLASS(ItemGroup),
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, measureRectCenter, QT_TR_NOOP("m"),
                QT_TR_NOOP("Center of the rect for measurements"), measureRectCenter, setMeasureRectCenter)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, measureRectSize, QT_TR_NOOP("m"),
                QT_TR_NOOP("Size of the rect for measurements"), measureRectSize, setMeasureRectSize)
    STEPCORE_PROPERTY_R_D(double, rectVolume, STEPCORE_FROM_UTF8("m²"),
                QT_TR_NOOP("Volume of the measureRect"), rectVolume)
    STEPCORE_PROPERTY_R_D(double, rectParticleCount, STEPCORE_UNITS_1,
                QT_TR_NOOP("Count of particles in the measureRect"), rectParticleCount)
    STEPCORE_PROPERTY_R_D(double, rectConcentration, STEPCORE_FROM_UTF8("1/m²"),
                QT_TR_NOOP("Concentration of particles in the measureRect"), rectConcentration)
    STEPCORE_PROPERTY_R_D(double, rectPressure, QT_TR_NOOP("Pa"),
                QT_TR_NOOP("Pressure of particles in the measureRect"), rectPressure)
    STEPCORE_PROPERTY_R_D(double, rectTemperature, QT_TR_NOOP("K"),
                QT_TR_NOOP("Temperature of particles in the measureRect"), rectTemperature)
    STEPCORE_PROPERTY_R_D(double, rectMeanKineticEnergy, QT_TR_NOOP("J"),
                QT_TR_NOOP("Mean kinetic energy of particles in the measureRect"), rectMeanKineticEnergy)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, rectMeanVelocity, QT_TR_NOOP("m/s"),
                QT_TR_NOOP("Mean velocity of particles in the measureRect"), rectMeanVelocity)
    STEPCORE_PROPERTY_R_D(double, rectMeanParticleMass, QT_TR_NOOP("kg"),
                QT_TR_NOOP("Mean mass of particles in the measureRect"), rectMeanParticleMass)
    STEPCORE_PROPERTY_R_D(double, rectMass, QT_TR_NOOP("kg"),
                QT_TR_NOOP("Total mass of particles in the measureRect"), rectMass)
    )

STEPCORE_META_OBJECT(GasErrors, QT_TR_NOOP("Errors class for Gas"), 0, STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_R_D(double, rectPressureVariance, QT_TR_NOOP("Pa"),
                QT_TR_NOOP("Variance of pressure of particles in the measureRect"), rectPressureVariance)
    STEPCORE_PROPERTY_R_D(double, rectTemperatureVariance, QT_TR_NOOP("K"),
                QT_TR_NOOP("Variance of temperature of particles in the measureRect"), rectTemperatureVariance)
    STEPCORE_PROPERTY_R_D(double, rectMeanKineticEnergyVariance, QT_TR_NOOP("J"),
                QT_TR_NOOP("Variance of mean kinetic energy of particles in the measureRect"), rectMeanKineticEnergyVariance)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, rectMeanVelocityVariance, QT_TR_NOOP("m/s"),
                QT_TR_NOOP("Variance of mean velocity of particles in the measureRect"), rectMeanVelocityVariance)
    STEPCORE_PROPERTY_R_D(double, rectMeanParticleMassVariance, QT_TR_NOOP("kg"),
                QT_TR_NOOP("Variance of mean mass of particles in the measureRect"), rectMeanParticleMassVariance)
    STEPCORE_PROPERTY_R_D(double, rectMassVariance, QT_TR_NOOP("kg"),
                QT_TR_NOOP("Variance of total mass of particles in the measureRect"), rectMassVariance)
    )

GasLJForce* GasLJForceErrors::gasLJForce() const
{
    return static_cast<GasLJForce*>(owner());
}

GasLJForce::GasLJForce(double depth, double rmin, double cutoff)
    : _depth(depth), _rmin(rmin), _cutoff(cutoff)
{
    calcABC();
}

void GasLJForce::calcABC()
{
    double m = 12*_depth;
    _rmin6 = pow(_rmin, 6);
    _rmin12 = _rmin6*_rmin6;
    _a = m*_rmin12; _b = m*_rmin6;
    _c = _cutoff*_cutoff;
}

void GasLJForce::calcForce(bool calcVariances)
{
    if(!group()) return;

    // NOTE: Currently we are handling only children of the same group
    const ItemList::const_iterator end = group()->items().end();
    for(ItemList::const_iterator i1 = group()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        for(ItemList::const_iterator i2 = i1+1; i2 != end; ++i2) {
            if(!(*i2)->metaObject()->inherits<GasParticle>()) continue;

            GasParticle* p1 = static_cast<GasParticle*>(*i1);
            GasParticle* p2 = static_cast<GasParticle*>(*i2);
            Vector2d r = p2->position() - p1->position();
            double rnorm2 = r.norm2();
            if(rnorm2 < _c) {
                double rnorm6 = rnorm2*rnorm2*rnorm2;
                double rnorm8 = rnorm6*rnorm2;
                Vector2d force = r * ((_a/rnorm6 - _b)/rnorm8);
                p2->applyForce(force);
                force.invert();
                p1->applyForce(force);

                if(calcVariances) {
                    ParticleErrors* pe1 = p1->particleErrors();
                    ParticleErrors* pe2 = p2->particleErrors();
                    Vector2d rV = pe2->positionVariance() + pe1->positionVariance();

                    GasLJForceErrors* ge = gasLJForceErrors();
                    Vector2d forceV = r.cSquare() * (
                        ge->_rminVariance * square( (12*_a/_rmin/rnorm6 - 6*_b/_rmin)/rnorm8 ) +
                        ge->_depthVariance * square( 12*(_rmin12/rnorm6 - _rmin6)/rnorm8 ) );

                    forceV[0] += rV[0] * square( (_a/rnorm6*( 1 - 14*r[0]*r[0]/rnorm2 ) -
                                                  _b*( 1 - 8*r[0]*r[0]/rnorm2 ))/rnorm8 ) +
                                 rV[1] * square( (_a/rnorm6*14 - _b*8)*r[0]*r[1]/(rnorm8*rnorm2) );
                    forceV[1] += rV[1] * square( (_a/rnorm6*( 1 - 14*r[1]*r[1]/rnorm2 ) -
                                                  _b*( 1 - 8*r[1]*r[1]/rnorm2 ))/rnorm8 ) +
                                 rV[0] * square( (_a/rnorm6*14 - _b*8)*r[0]*r[1]/(rnorm8*rnorm2) );

                    pe1->applyForceVariance(forceV);
                    pe2->applyForceVariance(forceV);
                }
            }
        }
    }
}

Gas* GasErrors::gas() const
{
    return static_cast<Gas*>(owner());
}

double Gas::rectVolume() const
{
    return _measureRectSize[0]*_measureRectSize[1];
}

double Gas::randomUniform(double min, double max)
{
    return double(std::rand())/RAND_MAX*(max-min) + min;
}

double Gas::randomGauss(double mean, double deviation)
{
    // Polar Box-Muller algorithm
    double x1, x2, w;
 
    do {
        x1 = 2.0 * double(std::rand())/RAND_MAX - 1.0;
        x2 = 2.0 * double(std::rand())/RAND_MAX - 1.0;
        w = x1 * x1 + x2 * x2;
    } while( w >= 1.0 || w == 0 );

    w = sqrt( (-2.0 * log( w ) ) / w );
    return x1 * w * deviation + mean;
}

GasParticleList Gas::rectCreateParticles(int count,
                                double mass, double temperature,
                                const Vector2d& meanVelocity)
{
    GasParticleList particles;

    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;
    double deviation = std::sqrt( Constants::Boltzmann * temperature / mass );

    for(int i=0; i<count; ++i) {
        Vector2d pos(randomUniform(r0[0], r1[0]), randomUniform(r0[1], r1[1]));
        Vector2d vel(randomGauss(meanVelocity[0], deviation), randomGauss(meanVelocity[1], deviation));
        GasParticle* particle = new GasParticle(pos, vel, mass);
        particles.push_back(particle);
    }

    return particles;
}

void Gas::addParticles(const GasParticleList& particles)
{
    const GasParticleList::const_iterator end = particles.end();
    for(GasParticleList::const_iterator it = particles.begin(); it != end; ++it) {
        addItem(*it);
    }
}

double Gas::rectParticleCount() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        ++count;
    }

    return count;
}

double Gas::rectMeanParticleMass() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    double mass = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        mass += p1->mass();
        ++count;
    }

    mass /= count;
    return mass;
}

double GasErrors::rectMeanParticleMassVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double count = 0;

    double mass = gas()->rectMeanParticleMass();
    double massVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        massVariance += square(p1->mass() - mass);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) massVariance += pe1->massVariance();

        ++count;
    }

    massVariance /= square(count);
    return massVariance;
}

double Gas::rectMass() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double mass = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        mass += p1->mass();
    }

    return mass;
}

double GasErrors::rectMassVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double massVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) massVariance += pe1->massVariance();
    }

    return massVariance;
}

double Gas::rectConcentration() const
{
    return rectParticleCount() / rectVolume();
}

Vector2d Gas::rectMeanVelocity() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    Vector2d velocity(0);

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        velocity += p1->velocity();
        ++count;
    }

    velocity /= count;
    return velocity;
}

Vector2d GasErrors::rectMeanVelocityVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double count = 0;

    Vector2d velocity = gas()->rectMeanVelocity();
    Vector2d velocityVariance(0);

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        velocityVariance += (p1->velocity() - velocity).cSquare(); 

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) velocityVariance += pe1->velocityVariance();

        ++count;
    }

    velocityVariance /= square(count);
    return velocityVariance;
}

double Gas::rectMeanKineticEnergy() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    double energy = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        energy += p1->mass() * p1->velocity().norm2();
        ++count;
    }

    energy /= (2.0*count);
    return energy;
}

double GasErrors::rectMeanKineticEnergyVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double count = 0;

    double energy = 2*gas()->rectMeanKineticEnergy();
    double energyVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        double pEnergy = p1->mass() * p1->velocity().norm2();
        energyVariance += square(pEnergy - energy);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) {
            energyVariance +=
                pe1->massVariance() * square(p1->velocity().norm2()) +
                pe1->velocityVariance().innerProduct(
                    (2*p1->mass()*p1->velocity()).cSquare() );
        }

        ++count;
    }

    energyVariance /= square(2.0*count);
    return energyVariance;
}

double Gas::rectTemperature() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    double temperature = 0;

    StepCore::Vector2d meanVelocity = rectMeanVelocity();
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        temperature += p1->mass() * (p1->velocity() - meanVelocity).norm2();
        ++count;
    }

    // no 3/2 factor since we live in 2d
    temperature /= (2.0*count*Constants::Boltzmann);
    return temperature;
}

double GasErrors::rectTemperatureVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double count = 0;

    StepCore::Vector2d meanVelocity = gas()->rectMeanVelocity();
    double temperature = 2.0*Constants::Boltzmann*gas()->rectTemperature();
    double temperatureVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        double pTemperature = p1->mass() * (p1->velocity() - meanVelocity).norm2();
        temperatureVariance += square(pTemperature - temperature);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) {
            temperatureVariance +=
                pe1->massVariance() * square((p1->velocity() - meanVelocity).norm2()) +
                pe1->velocityVariance().innerProduct(
                    (p1->mass()*(p1->velocity() - meanVelocity)).cSquare() );
        }

        ++count;
    }

    temperatureVariance /= square(2.0*Constants::Boltzmann*count);
    // XXX: We could easily take into account BoltzmannError here
    // but this can confuse users so for now we don't do it
    
    return temperatureVariance;
}

// XXX: this formula is incorrect when forces are big
// XXX: use better formula (for example from lammps)
double Gas::rectPressure() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double pressure = 0;

    StepCore::Vector2d meanVelocity = rectMeanVelocity();
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        pressure += p1->mass() * (p1->velocity() - meanVelocity).norm2();
    }

    pressure /= (2.0 * rectVolume());
    return pressure;
}

double GasErrors::rectPressureVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    StepCore::Vector2d meanVelocity = gas()->rectMeanVelocity();
    double pressure = gas()->rectPressure();
    double pressureVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        double pPressure = p1->mass() * (p1->velocity() - meanVelocity).norm2();
        pressureVariance += square(pPressure - pressure);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) {
            pressureVariance +=
                pe1->massVariance() * square((p1->velocity() - meanVelocity).norm2()) +
                pe1->velocityVariance().innerProduct(
                    (p1->mass()*(p1->velocity() - meanVelocity)).cSquare() );
        }
    }

    pressureVariance /= square(2.0*gas()->rectVolume());
    return pressureVariance;
}

}


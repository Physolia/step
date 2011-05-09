/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   Step is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Step is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Step; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "worldfactory.h"
#include "worldgraphics.h"
#include "worldmodel.h"

#include <stepcore/world.h>
#include <stepcore/particle.h>
#include <stepcore/rigidbody.h>
#include <stepcore/gas.h>
#include <stepcore/softbody.h>
#include <stepcore/gravitation.h>
#include <stepcore/coulombforce.h>
#include <stepcore/spring.h>
#include <stepcore/motor.h>
#include <stepcore/gslsolver.h>
#include <stepcore/eulersolver.h>
#include <stepcore/collisionsolver.h>
#include <stepcore/constraintsolver.h>
#include <stepcore/joint.h>
#include <stepcore/tool.h>
#include <stepcore/types.h>

#include "particlegraphics.h"
#include "polygongraphics.h"
#include "gasgraphics.h"
#include "softbodygraphics.h"
#include "springgraphics.h"
#include "motorgraphics.h"
#include "jointgraphics.h"
#include "toolgraphics.h"

#include <QMessageBox>
#include <KIcon>
#include <KIconLoader>
#include <KDebug>

template<typename T>
ItemCreator* newItemCreatorHelper(const QString& className,
                    WorldModel* worldModel, WorldScene* worldScene)
{
    return new T(className, worldModel, worldScene);
}

template<typename T>
ItemCreator* newItemCreator3DHelper(const QString& className,
                    WorldModel* worldModel, WorldGLScene* worldGLScene)
{
    return new T(className, worldModel, worldGLScene);
}

template<typename T>
WorldGraphicsItem* newGraphicsItemHelper(StepCore::Item* item, WorldModel* worldModel)
{
    return new T(item, worldModel);
}

template<typename T>
ItemMenuHandler* newItemMenuHandlerHelper(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
{
    return new T(object, worldModel, parent);
}

template<typename T>
WorldGraphicsItem3D* newGraphicsItem3DHelper(StepCore::Item* item, WorldModel* worldModel)
{
    return new T(item, worldModel);
}

WorldFactory::WorldFactory()
{
    _nullIcon = new KIcon();

    #define ___REGISTER_EXT(Class, newGraphicsCreator, newGraphicsItem, newItemMenuHandler,newGraphicsItem3D) \
        static ExtMetaObject extMetaObject ## Class = \
                { newGraphicsCreator, newGraphicsItem, newItemMenuHandler, false,false}; \
        registerMetaObject(StepCore::Class::staticMetaObject()); \
        _extMetaObjects.insert(StepCore::Class::staticMetaObject(), &extMetaObject ## Class); \
        _orderedMetaObjects.push_back(QString(StepCore::Class::staticMetaObject()->className())); \
        loadIcon(StepCore::Class::staticMetaObject(), const_cast<ExtMetaObject*>(&extMetaObject ## Class))

    #define ___REGISTER_EXT_E(Class, newGraphicsCreator, newGraphicsItem, newItemMenuHandler,newGraphicsItem3D) \
        ___REGISTER_EXT(Class, newGraphicsCreator, newGraphicsItem, newItemMenuHandler,newGraphicsItem3D); \
        registerMetaObject(StepCore::Class##Errors::staticMetaObject())

    #define __REGISTER(Class) ___REGISTER_EXT(Class, NULL, NULL, NULL, NULL)
    #define __REGISTER_E(Class) ___REGISTER_EXT_E(Class, NULL, NULL, NULL, NULL)

    #define __REGISTER_EXT(Class, GraphicsCreator, GraphicsItem, ItemMenuHandler,GraphicsItem3D) \
        ___REGISTER_EXT(Class, newItemCreatorHelper<GraphicsCreator>, \
                   newGraphicsItemHelper<GraphicsItem>, newItemMenuHandlerHelper<ItemMenuHandler>,newItemCreator3DHelper<GraphicsItem3D>)

    #define __REGISTER_EXT_E(Class, GraphicsCreator, GraphicsItem, ItemMenuHandler,GraphicsItem3D) \
        __REGISTER_EXT(Class, GraphicsCreator, GraphicsItem, ItemMenuHandler,GraphicsItem3D); \
        registerMetaObject(StepCore::Class##Errors::staticMetaObject())

    #define __ADD_TO_PALETTE(Class) \
        _paletteMetaObjects.push_back(QString(StepCore::Class::staticMetaObject()->className()))

    #define __ADD_SEPARATOR \
        _paletteMetaObjects.push_back(QString())

    __REGISTER_E(Object);

    __REGISTER(Item);
    __REGISTER(World);
    __REGISTER(ItemGroup);
    __REGISTER(Body);
    __REGISTER(Force);
    __REGISTER(Solver);
    __REGISTER(CollisionSolver);
    __REGISTER(ConstraintSolver);

    __REGISTER_EXT_E(Particle, ItemCreator, ParticleGraphicsItem, ItemMenuHandler,false);
    __REGISTER_EXT_E(ChargedParticle, ItemCreator, ParticleGraphicsItem, ItemMenuHandler,false);

    __REGISTER_EXT(Disk, DiskCreator, DiskGraphicsItem, ItemMenuHandler,false);
    __REGISTER_EXT(Box, BoxCreator, BoxGraphicsItem, ItemMenuHandler, BoxGraphicsItem3D);
    __REGISTER_EXT(Polygon, PolygonCreator, PolygonGraphicsItem, ItemMenuHandler,false);

    __REGISTER_EXT(GasParticle, ItemCreator, ParticleGraphicsItem, ItemMenuHandler,false);
    __REGISTER(GasLJForce);
    __REGISTER_EXT(Gas, GasCreator, GasGraphicsItem, GasMenuHandler,false);

    __REGISTER_EXT(SoftBodySpring, SpringCreator, SoftBodySpringGraphicsItem, ItemMenuHandler,false);
    __REGISTER_EXT(SoftBodyParticle, ItemCreator, SoftBodyParticleGraphicsItem, ItemMenuHandler,false);
    __REGISTER_EXT(SoftBody, SoftBodyCreator, SoftBodyGraphicsItem, SoftBodyMenuHandler,false);

    __REGISTER_EXT_E(Spring, SpringCreator, SpringGraphicsItem, ItemMenuHandler,false);
    __REGISTER_EXT(LinearMotor, LinearMotorCreator, LinearMotorGraphicsItem, ItemMenuHandler,false);
    __REGISTER_EXT(CircularMotor, CircularMotorCreator, CircularMotorGraphicsItem, ItemMenuHandler,false);

    __REGISTER_E(WeightForce);
    __REGISTER_E(GravitationForce);
    __REGISTER_E(CoulombForce);

    __REGISTER_EXT(Anchor, AnchorCreator, AnchorGraphicsItem, ItemMenuHandler,false);
    __REGISTER_EXT(Pin, PinCreator, PinGraphicsItem, ItemMenuHandler,false);
    __REGISTER_EXT(Stick, StickCreator, StickGraphicsItem, ItemMenuHandler,false);
    //__REGISTER_EXT(Rope, StickCreator, StickGraphicsItem, ItemMenuHandler);

    __REGISTER(EulerSolver);
    __REGISTER(AdaptiveEulerSolver);

    __REGISTER(GJKCollisionSolver);
    __REGISTER(CGConstraintSolver);

#ifdef STEPCORE_WITH_GSL
    __REGISTER(GslRK2Solver);
    __REGISTER(GslRK4Solver);
    __REGISTER(GslRKF45Solver);
    __REGISTER(GslRKCKSolver);
    __REGISTER(GslRK8PDSolver);
    __REGISTER(GslRK2IMPSolver);
    __REGISTER(GslRK4IMPSolver);

    __REGISTER(GslAdaptiveRK2Solver);
    __REGISTER(GslAdaptiveRK4Solver);
    __REGISTER(GslAdaptiveRKF45Solver);
    __REGISTER(GslAdaptiveRKCKSolver);
    __REGISTER(GslAdaptiveRK8PDSolver);
    __REGISTER(GslAdaptiveRK2IMPSolver);
    __REGISTER(GslAdaptiveRK4IMPSolver);
#endif

    __REGISTER(NoteImage);
    __REGISTER(NoteFormula);
    __REGISTER_EXT(Note, ItemCreator, NoteGraphicsItem, ItemMenuHandler,false);
    __REGISTER_EXT(Meter, ItemCreator, MeterGraphicsItem, MeterMenuHandler,false);
    __REGISTER_EXT(Graph, ItemCreator, GraphGraphicsItem, GraphMenuHandler,false);
    __REGISTER_EXT(Controller, ItemCreator, ControllerGraphicsItem, ControllerMenuHandler,false);
    __REGISTER_EXT(Tracer, TracerCreator, TracerGraphicsItem, TracerMenuHandler,false);

    // Palette
    __ADD_TO_PALETTE(Particle);
    __ADD_TO_PALETTE(ChargedParticle);
    __ADD_TO_PALETTE(Disk);
    __ADD_TO_PALETTE(Box);
    __ADD_TO_PALETTE(Polygon);
    __ADD_SEPARATOR;

    __ADD_TO_PALETTE(Spring);
    __ADD_TO_PALETTE(LinearMotor);
    __ADD_TO_PALETTE(CircularMotor);
    __ADD_SEPARATOR;

    __ADD_TO_PALETTE(Gas);
    __ADD_TO_PALETTE(SoftBody);
    __ADD_SEPARATOR;

    __ADD_TO_PALETTE(WeightForce);
    __ADD_TO_PALETTE(GravitationForce);
    __ADD_TO_PALETTE(CoulombForce);
    __ADD_SEPARATOR;

    __ADD_TO_PALETTE(Anchor);
    __ADD_TO_PALETTE(Pin);
    __ADD_TO_PALETTE(Stick);
    //__ADD_TO_PALETTE(Rope);
    __ADD_SEPARATOR;

    __ADD_TO_PALETTE(Note);
    __ADD_TO_PALETTE(Meter);
    __ADD_TO_PALETTE(Tracer);
    __ADD_TO_PALETTE(Graph);
    __ADD_TO_PALETTE(Controller);
}

WorldFactory::~WorldFactory()
{
    delete _nullIcon; 
}

ItemCreator* WorldFactory::newItemCreator(const QString& className,
                    WorldModel* worldModel, WorldScene* worldScene) const
{
    const StepCore::MetaObject* mObject = metaObject(className);
    if(!mObject) return NULL;
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(mObject, NULL);
    if(extMetaObject && extMetaObject->newItemCreator)
        return extMetaObject->newItemCreator(className, worldModel, worldScene);
    else return new ItemCreator(className, worldModel, worldScene);
}

ItemCreator* WorldFactory::newItemCreator3D(const QString& className,
                    WorldModel* worldModel, WorldGLScene* worldGLScene) const
{
    const StepCore::MetaObject* mObject = metaObject(className);
    if(!mObject) return NULL;
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(mObject, NULL);
    if(extMetaObject && extMetaObject->newItemCreator3D)
        return extMetaObject->newItemCreator3D(className, worldModel, worldGLScene);
    else return new ItemCreator(className, worldModel, worldGLScene);
}

WorldGraphicsItem* WorldFactory::newGraphicsItem(StepCore::Item* item, WorldModel* worldModel) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(item->metaObject(), NULL);
    if(extMetaObject && extMetaObject->newGraphicsItem)
        return extMetaObject->newGraphicsItem(item, worldModel);
    return NULL;
}
WorldGraphicsItem3D* WorldFactory::newGraphicsItem3D(StepCore::Item* item, WorldModel* worldModel) const
{
     const ExtMetaObject *extMetaObject = _extMetaObjects.value(item->metaObject(), NULL);
     if(extMetaObject && extMetaObject->newGraphicsItem3D)
         return extMetaObject->newGraphicsItem3D(item, worldModel);
     return NULL;
}


ItemMenuHandler* WorldFactory::newItemMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(object->metaObject(), NULL);
    if(extMetaObject && extMetaObject->newItemMenuHandler)
        return extMetaObject->newItemMenuHandler(object, worldModel, parent);
    else return new ItemMenuHandler(object, worldModel, parent);
}

bool WorldFactory::hasObjectIcon(const StepCore::MetaObject* mObject) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(mObject, NULL);
    if(extMetaObject && extMetaObject->hasIcon) return true;
    else return false;
}

const KIcon& WorldFactory::objectIcon(const StepCore::MetaObject* mObject) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(mObject, NULL);
    if(extMetaObject && extMetaObject->icon) return *(extMetaObject->icon);
    else {
        qWarning("Trying to load icon for unregistered metaObject\n");
        return *_nullIcon;
    }
}

void WorldFactory::loadIcon(const StepCore::MetaObject* metaObject, ExtMetaObject* extMetaObject)
{
    QString iconName = QString("step_object_") + metaObject->className();
    extMetaObject->icon = new KIcon(iconName);
    QString iconPath = KIconLoader::global()->iconPath(iconName, KIconLoader::Small, true);
    extMetaObject->hasIcon = !iconPath.isEmpty();
}


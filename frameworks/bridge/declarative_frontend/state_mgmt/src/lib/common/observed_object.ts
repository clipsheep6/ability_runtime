/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/**
 * ObservedObject, i.e. wrapper for class Object type state variable
 *
 * This class and all other definitoons in this file are framework
 * internal / non-SDK
 *
 */


// class of Class in TS
// ad defined in @angular/core
interface Type<T> extends Function {
  new(...args: any[]): T;
}

/**
* @Observed Decorator function, use
*    @Observed class ClassA { ... }
* when defining ClassA
*
* Can also be used to create a new Object and wrap it in
* ObservedObject by calling
*   obsObj = Observed(ClassA)(params to ClassA constructor)
*
* Note this works only for classes, not for ClassA[]
* In that case use factory function
*   obsObj = ObservedObject.createNew<ClassA[]>([])
*/

const Observed: (obj) => any = function () {
    return function Observed(target: any): any {
      const IS_PROXIED = Symbol('___is_proxied___');
      stateMgmtConsole.debug(`@Observed: define ${target.name} extended`);
      const Observed = class extends target {
        constructor(...args) {
          super(...args);
          let isProxied = this.IS_PROXIED;
          Object.defineProperty(this, IS_PROXIED, {value: true});
          return isProxied
            ? this 
            : ObservedObject.createNew(this, null);
        }
      };
      return Observed;
    }
}()

class SubscribableHandler {
  static IS_OBSERVED_OBJECT = Symbol("_____is_observed_object__");
  static RAW_OBJECT = Symbol("_____raw_object__");
  static SUBSCRIBE = Symbol("_____subscribe__");
  static UNSUBSCRIBE = Symbol("_____unsubscribe__")

  private owningProperties_: Set<number>

  constructor(owningProperty: IPropertySubscriber) {
    this.owningProperties_ = new Set<number>();
    if (owningProperty) {
      this.addOwningProperty(owningProperty);
    }
    stateMgmtConsole.debug(`SubscribableHandler: construcstor done`);
  }

  addOwningProperty(subscriber: IPropertySubscriber): void {
    if (subscriber) {
        stateMgmtConsole.debug(`SubscribableHandler: addOwningProperty: subscriber '${subscriber.id__()}'.`)
        this.owningProperties_.add(subscriber.id__());
    } else {
        stateMgmtConsole.warn(`SubscribableHandler: addOwningProperty: undefined subscriber. - Internal error?`);
    }
  }

  /*
      the inverse function of createOneWaySync or createTwoWaySync
    */
  public removeOwningProperty(property: IPropertySubscriber): void {
    return this.removeOwningPropertyById(property.id__());
  }

  public removeOwningPropertyById(subscriberId: number): void {
    stateMgmtConsole.debug(`SubscribableHandler: removeOwningProperty '${subscriberId}'.`)
    this.owningProperties_.delete(subscriberId);
  }


  protected notifyPropertyHasChanged(propName: string, newValue: any) {
    stateMgmtConsole.debug(`SubscribableHandler: notifyPropertyHasChanged '${propName}'.`)
    this.owningProperties_.forEach((subscribedId) => {
      var owningProperty: IPropertySubscriber = SubscriberManager.Find(subscribedId)
      if (owningProperty) {
        if ('hasChanged' in owningProperty) {
          (owningProperty as ISinglePropertyChangeSubscriber<any>).hasChanged(newValue);
        }
        if ('propertyHasChanged' in owningProperty) {
          (owningProperty as IMultiPropertiesChangeSubscriber).propertyHasChanged(propName);
        }
      } else {
        stateMgmtConsole.warn(`SubscribableHandler: notifyHasChanged: unknown subscriber.'${subscribedId}' error!.`);
      }
    });
  }

  public get(target: Object, property: PropertyKey): any {
    return (property === SubscribableHandler.IS_OBSERVED_OBJECT) ? true :
      (property === SubscribableHandler.RAW_OBJECT) ? target : target[property];
  }

  public set(target: Object, property: PropertyKey, newValue: any): boolean {
    switch (property) {
      case SubscribableHandler.SUBSCRIBE:
        // assignment obsObj[SubscribableHandler.SUBSCRCRIBE] = subscriber
        this.addOwningProperty(newValue as IPropertySubscriber);
        return true;
        break;
      case SubscribableHandler.UNSUBSCRIBE:
        // assignment obsObj[SubscribableHandler.UN_SUBSCRCRIBE] = subscriber
        this.removeOwningProperty(newValue as IPropertySubscriber);
        return true;
        break;
      default:
        if (target[property] == newValue) {
          return true;
        }
        target[property] = newValue;
        this.notifyPropertyHasChanged(property.toString(), newValue);
        return true;
        break;
    }

    // unreachable
    return false;
  }
}


class ExtendableProxy {
  constructor(obj: Object, handler: SubscribableHandler) {
    return new Proxy(obj, handler);
  }
}

class ObservedObject<T extends Object> extends ExtendableProxy {

  /**
   * Factory function for ObservedObjects /
   *  wrapping of objects for proxying
   *
   * @param rawObject unproxied Object or ObservedObject
   * @param objOwner owner of this Object to sign uop for propertyChange
   *          notifications
   * @returns the rawObject if object is already an ObservedObject,
   *          otherwise the newly created ObservedObject
   */
  public static createNew<T extends Object>(rawObject: T,
    owningProperty: IPropertySubscriber): any {

    if (rawObject === null || rawObject === undefined) {
      stateMgmtConsole.error(`ObservedObject.CreateNew, input object must not be null or undefined.`);
      return null;
    }

    if (ObservedObject.IsObservedObject(rawObject)) {
      ObservedObject.addOwningProperty(rawObject, owningProperty);
      return rawObject;
    } else {
      return new ObservedObject<T>(rawObject, owningProperty);
    }
  }

  /*
    Return the unproxied object 'inside' the ObservedObject / the ES6 Proxy
    no set observation, no notification of changes!
    Use with caution, do not store any references
  */
  static GetRawObject(obj: any): any {
    return !ObservedObject.IsObservedObject(obj) ? obj : obj[SubscribableHandler.RAW_OBJECT];
  }

  /**
   *
   * @param obj anything
   * @returns true if the parameter is an Object wrpped with a ObservedObject
   * Note: Since ES6 Proying is transparent, 'instance of' will not work. Use
   * this static function instead.
   */
  static IsObservedObject(obj: any): boolean {
    return obj ? (obj[SubscribableHandler.IS_OBSERVED_OBJECT] === true) : false;
  }

  static addOwningProperty(obj: Object, subscriber: IPropertySubscriber): boolean {
    if (!ObservedObject.IsObservedObject(obj)) {
      return false;
    }

    obj[SubscribableHandler.SUBSCRIBE] = subscriber;
    return true;
  }

  static removeOwningProperty(obj: Object,
    subscriber: IPropertySubscriber): boolean {
    if (!ObservedObject.IsObservedObject(obj)) {
      return false;
    }

    obj[SubscribableHandler.UNSUBSCRIBE] = subscriber;
    return true;
  }

  /**
   * Create a new ObservableObject and subscribe its owner to propertyHasChanged
   * ntifications
   * @param obj  raw Object, if obj is a ObservableOject throws an error
   * @param objectOwner
   */
  constructor(obj: T, objectOwningProperty: IPropertySubscriber) {
    if (ObservedObject.IsObservedObject(obj)) {
      throw new Error("Invalid constructor argument error: ObservableObject contructor called with an ObservedObject as parameer");
    }
    let handler = new SubscribableHandler(objectOwningProperty);
    super(obj, handler);

    if (ObservedObject.IsObservedObject(obj)) {
      stateMgmtConsole.error("ObservableOject constructor: INTERNAL ERROR: after jsObj is observedObject already");
    }
  } // end of constructor

}

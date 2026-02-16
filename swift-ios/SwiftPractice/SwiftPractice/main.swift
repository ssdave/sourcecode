//
//  main.swift
//  SwiftPractice
//
//  Created by Sanket Dave on 2/16/26.
//

import Foundation

class car {
    var color: String
    var make: String
    var year: Int
    
    init(color: String, make: String, year: Int) {
        self.color = color
        self.make = make
        self.year = year
    }
}

class bike: car {
    var wheels: Int
    
    init(color: String, make: String, year: Int, wheels: Int) {
        self.wheels = wheels
        super.init(color: color, make: make, year: year)
    }
}

let myCar = car(color: "Red", make: "Toyota", year: 2020)
let myBike = bike(color: "Blue", make: "Honda", year: 2019, wheels: 2)

print("The color of my car is \(myCar.color)")
print("The make of my car is \(myCar.make)")

var born: Int? = nil
print(born!)

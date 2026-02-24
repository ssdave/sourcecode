//
//  LibraryViewController.swift
//  UIKitPractice
//
//  Created by Sanket Dave on 2/23/26.
//

import Foundation
import UIKit

class LibraryViewController: UIViewController {
    
    override func viewDidLoad() {
        super.viewDidLoad()

    }
    

}

/*
extension LibraryViewController: UITableViewDataSource {

    func numberOfSections(in tableView: UITableView) -> Int {
        return 3
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        var rows = 0
        if( section == 0 ) {
            rows = 3
        } else if ( section == 1 ) {
            rows = 2
        } else if ( section == 2 ) {
            rows = 10
        }
        return rows
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "libraryCell", for: indexPath)
        cell.textLabel?.text = "Row \(indexPath.row)"
        return cell
    }
}*/



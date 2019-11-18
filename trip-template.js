const tripTemplateBase = {
    id: "packageid",
    name: "dets.one",
    address: {
        addr_line: {
            addr_line_1: "dets.two",
            addr_line_2: "dets.three",
            zip: "(zipper, zipcode.code, zipcode.plus4)"
        },
        city_st: "Tmplate Main St",
    },
    item_evaluation: {
        becomes_null: "dets.is_a_null",
        nested_bool: "dets.is_true",
        nested_num: "zipcode.as_num",
        hardcoded_bool: true,
        hardcoded_num: 5,
        other: "dets.nonexistent",
        not_found: "anarray[1]"
    },
    description: "description",
    onehundred_from_an_array: "AnArray[0]",
    fifty_nested: "NestedArray.ItemZero[1].fifty",
    legged_people: { ArrayOf: { IterateOver: "XForm_array", ArrayTemplate: {
            person_name: "Name",
            leg_status: {ArrayOf: { IterateOver: "Leg_Status_Array.leg", ArrayTemplate: {
                    base_sample: "_base.Description",
                    orientation: "name",
                    foot_status: "has_foot",
                    toes: {
                        count: "toes"
                    }
                }
            }}
        }
    }}
};

const inputData = {
    packageid: 123400,
    dets: {
        one: "Super-Duper-Package",
        two: "6000 Old Park Lane",
        three: "Orlando, FL",
        is_a_null: null,
        is_true: true
    },
    zipcode: {
        code: "32819",
        plus4: "9010",
        as_num: 32819
    },
    description: "Super.Duper.New.Experience",
    AnArray: [ 100, 200, 300],
    NestedArray : {
        ItemZero: [ {fifty: "25"}, {fifty: "50"} ]
    },
    XForm_array: [
        {
            Name: "Person_One",
            Leg_Status_Array: {
                count: 2,
                leg: [
                    { name: "left", has_foot: true, toes: 5 },
                    { name: "right", has_foot: false, toes: 0 }
                ]
            }
        },
        {
            Name: "Person Two",
            count: 2,
            Leg_Status_Array: {
                leg: [
                    { name: "left", has_foot: true, toes: 4 },
                    { name: "right", has_foot: true, toes: 5 }
                ]
            }
        }
    ]
};

const xfuncts = {
    zipper : (left, right)=> {
        if (right) {
            return left+"-"+right;
        } else {
            return left;
        }
      }
}

module.exports = {tripTemplateBase, inputData, xfuncts};

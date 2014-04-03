<?php

/**
 * Implements hook_default_rules_configuration().
 */
function commerce_symp_ukmail_default_rules_configuration() {
  $configs = array();

  // Add components for extra shipping weight
  $components = array(
    array(
      'cost' => 50, // pence
      'max_weight' => 20, // kg
    ),
    array(
      'cost' => 112, // pence
      'max_weight' => 20, // kg
    ),
    array(
      'cost' => 125, // pence
      'max_weight' => 20, // kg
    ),
  );

  foreach ($components as $component) {
    $label = 'calculated_line_item_weight_cost_'. $component['cost'] .'p_per_kilo';
    $rule_string = '{ "'. $label .'" : {
    "LABEL" : "Calculate shipping amount by weight of line item ('. $component['cost'] .'p per kilo)",
    "PLUGIN" : "rule",
    "OWNER" : "rules",
    "REQUIRES" : [ "rules", "commerce", "commerce_physical" ],
    "USES VARIABLES" : {
      "line_item" : { "label" : "Line item", "type" : "commerce_line_item" },
      "shipping_for_line_item" : {
        "label" : "Shipping cost for line item",
        "type" : "decimal",
        "parameter" : false
      }
    },
    "IF" : [
      { "entity_has_field" : { "entity" : [ "line-item" ], "field" : "commerce_product" } },
      { "entity_has_field" : { "entity" : [ "line-item:commerce-product" ], "field" : "field_weight" } },
      { "entity_exists" : {
          "type" : "commerce_order",
          "property" : "order_id",
          "value" : [ "line-item:order-id" ]
        }
      },
      { "commerce_physical_rules_order_weight_comparison" : {
          "commerce_order" : [ "line-item:order" ],
          "operator" : "\u003E=",
          "value" : "'. $component['max_weight'] .'",
          "unit" : "kg"
        }
      }
    ],
    "DO" : [
      { "data_calc" : {
          "USING" : {
            "input_1" : [ "line-item:commerce-product:field-weight:weight" ],
            "op" : "\/",
            "input_2" : "1000"
          },
          "PROVIDE" : { "result" : { "weight_in_kg" : "Weight in KG" } }
        }
      },
      { "data_calc" : {
          "USING" : {
            "input_1" : [ "weight-in-kg" ],
            "op" : "*",
            "input_2" : [ "line-item:quantity" ]
          },
          "PROVIDE" : { "result" : { "total_weight_line_item" : "Total weight of line item" } }
        }
      },
      { "data_calc" : {
          "USING" : { "input_1" : [ "total-weight-line-item" ], "op" : "*", "input_2" : "'. $component['cost'] .'" },
          "PROVIDE" : { "result" : { "line_item_calculated_shipping" : "Calculated shipping cost" } }
        }
      },
      { "data_calc" : {
          "USING" : {
            "input_1" : [ "line-item-calculated-shipping" ],
            "op" : "\/",
            "input_2" : "100"
          },
          "PROVIDE" : { "result" : { "convert_to_pounds" : "Convert to pounds" } }
        }
      },
      { "data_calc" : {
          "USING" : { "input_1" : [ "convert-to-pounds" ], "op" : "*", "input_2" : "2" },
          "PROVIDE" : { "result" : { "multiply_by_2" : "Multiple by 2" } }
        }
      },
      { "data_convert" : {
          "USING" : {
            "type" : "integer",
            "value" : [ "multiply-by-2" ],
            "rounding_behavior" : "up"
          },
          "PROVIDE" : { "conversion_result" : { "rounded_value" : "Rounded value" } }
        }
      },
      { "data_calc" : {
          "USING" : { "input_1" : [ "rounded-value" ], "op" : "\/", "input_2" : "2" },
          "PROVIDE" : { "result" : { "divide_by_2" : "Divide by 2" } }
        }
      },
      { "data_calc" : {
          "USING" : { "input_1" : [ "divide-by-2" ], "op" : "*", "input_2" : "100" },
          "PROVIDE" : { "result" : { "convert_to_pence" : "Convert back to pence" } }
        }
      },
      { "data_set" : {
          "data" : [ "shipping-for-line-item" ],
          "value" : [ "convert-to-pence" ]
        }
      }
    ],
    "PROVIDES VARIABLES" : [ "shipping_for_line_item" ]
  }
}';

    $configs[$label] = rules_import($rule_string);
  }

  $zones = array(
    'A' => 50,
    'B' => 50,
    'C' => 112,
    'D' => 125,
    'E' => 125,
  );
  foreach ($zones as $zone => $cost) {
    $label = 'extra_shipping_weight_cost_over_20kg_zone_'. strtolower($zone);
    $subtract = variable_get('commerce_symp_ukmail_ukmail_over_10kg_zone_'. strtolower($zone), '');
    $rule_string = '{ "'. $label .'" : {
    "LABEL" : "UK Mail - Extra shipping charge over 20kg - Zone '. $zone .'",
    "PLUGIN" : "reaction rule",
    "OWNER" : "rules",
    "REQUIRES" : [
      "rules",
      "commerce_physical",
      "commerce_line_item",
      "commerce_shipping"
    ],
    "ON" : { "commerce_shipping_calculate_rate" : [] },
    "IF" : [
      { "data_is" : { "data" : [ "commerce-line-item:type" ], "value" : "shipping" } },
      { "data_is" : {
          "data" : [ "commerce-line-item:commerce-shipping-service" ],
          "value" : "ukmail_over_10kg_zone_'. strtolower($zone) .'"
        }
      },
      { "commerce_physical_rules_order_weight_comparison" : {
          "commerce_order" : [ "commerce-line-item:order" ],
          "operator" : "\u003E",
          "value" : "20",
          "unit" : "kg"
        }
      }
    ],
    "DO" : [
      { "LOOP" : {
          "USING" : { "list" : [ "commerce-line-item:order:commerce-line-items" ] },
          "ITEM" : { "line_item" : "Line item" },
          "DO" : [
            { "component_calculated_line_item_weight_cost_'. $cost .'p_per_kilo" : {
                "USING" : { "line_item" : [ "line_item" ] },
                "PROVIDE" : { "shipping_for_line_item" : { "shipping_for_line_item" : "Shipping cost for line item" } }
              }
            },
            { "commerce_line_item_unit_price_add" : {
                "commerce_line_item" : [ "commerce-line-item" ],
                "amount" : [ "shipping-for-line-item" ],
                "component_name" : "shipping",
                "round_mode" : "1"
              }
            }
          ]
        }
      },
      { "commerce_line_item_unit_price_subtract" : {
          "commerce_line_item" : [ "commerce_line_item" ],
          "amount" : "'. ($subtract * 100) .'",
          "component_name" : "shipping",
          "round_mode" : "1"
        }
      }
    ]
  }
}';

    $configs[$label] = rules_import($rule_string);
  }

  return $configs;
}

/**
 * Implements hook_default_rules_configuration_alter().
 */
function commerce_symp_ukmail_default_rules_configuration_alter(&$configs) {
  // UK shipping rules
  $zones = range('A', 'E');

  $weight_brackets = array(
    array(
      'label' => 'Up to 1KG',
      'min' => FALSE,
      'max' => 1,
    ),
    array(
      'label' => 'Up to 2KG',
      'min' => 1,
      'max' => 2,
    ),
    array(
      'label' => 'Up to 5KG',
      'min' => 2,
      'max' => 5,
    ),
    array(
      'label' => 'Up to 10KG',
      'min' => 5,
      'max' => 10,
    ),
    array(
      'label' => 'Over 10KG',
      'min' => 10,
      'max' => FALSE,
    ),
  );

  foreach ($zones as $zone) {
    foreach ($weight_brackets as $weight_bracket) {
      $label = 'ukmail_'. str_replace(' ', '_', strtolower($weight_bracket['label'])) .'_zone_'. strtolower($zone);
      if (isset($configs['commerce_shipping_service_'. $label])) {
        $name = t('UKMail - @weight - Zone @zone', array('@weight' => $weight_bracket['label'], '@zone' => $zone));
        $weight_rules = array();
        if ($weight_bracket['min']) {
          $weight_rules[] = '{ "commerce_physical_rules_order_weight_comparison" : {
              "commerce_order" : [ "commerce_order" ],
              "operator" : "\u003E=",
              "value" : "'. $weight_bracket['min'] .'",
              "unit" : "kg"
            }
          }';
        }
        if ($weight_bracket['max']) {
          $weight_rules[] = '{ "commerce_physical_rules_order_weight_comparison" : {
              "commerce_order" : [ "commerce_order" ],
              "operator" : "\u003C",
              "value" : "'. $weight_bracket['max'] .'",
              "unit" : "kg"
            }
          }';
        }

        $rule_string = '{ "commerce_shipping_service_'. $label .'" : {
      "LABEL" : "'. $name .'",
      "PLUGIN" : "rule",
      "OWNER" : "rules",
      "REQUIRES" : [ "commerce_symp_ukmail", "commerce_shipping" ],
      "USES VARIABLES" : { "commerce_order" : { "type" : "commerce_order", "label" : "Order" } },
      "IF" : [
        { "AND" : [
            { "commerce_symp_ukmail_order_shipping_zone" : {
                "commerce_order" : [ "commerce_order" ],
                "address_field" : "commerce_customer_shipping|commerce_customer_address",
                "value" : "'. $zone .'"
              }
            },
            { "AND" : ['. implode(',', $weight_rules) .'] }
          ]
        }
      ],
      "DO" : [
        { "commerce_shipping_service_rate_order" : {
            "shipping_service_name" : "'. $label .'",
            "commerce_order" : [ "commerce-order" ]
          }
        }
      ]
    }
  }';
        $configs['commerce_shipping_service_'. $label] = rules_import($rule_string);
      }
    }
  }
}
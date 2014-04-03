<?php

function commerce_symp_ukmail_config_form() {

  // Set up UK first
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
  $form['uk_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('UK settings'),
  );
  foreach ($zones as $zone) {
    $form['uk_settings']['zone_'. $zone] = array(
      '#type' => 'fieldset',
      '#title' => t('Zone @zone', array('@zone' => $zone)),
    );
    foreach ($weight_brackets as $weight_bracket) {
      $name = t('UKMail - @weight - Zone @zone', array('@weight' => $weight_bracket['label'], '@zone' => $zone));
      $label = 'ukmail_'. str_replace(' ', '_', strtolower($weight_bracket['label'])) .'_zone_'. strtolower($zone);
      $form['uk_settings']['zone_'. $zone]['commerce_symp_ukmail_'. $label] = array(
        '#type' => 'textfield',
        '#title' => t('Price'),
        '#description' => t('Price for @name', array('@name' => $name)),
        '#size' => 5,
        '#field_prefix' => '&pound;',
        '#default_value' => variable_get('commerce_symp_ukmail_'. $label, ''),
      );
    }
  }

  return system_settings_form($form);
}
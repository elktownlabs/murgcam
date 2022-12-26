<template>
  <v-container>
    <h2 class="py-4">Modify Camera Settings</h2>
    <v-alert v-model="alert_error" border="top" color="red lighten-2" dark>{{ alert_error_text }}</v-alert>
    <v-alert v-model="alert_success" border="top" color="green lighten-2" show dismissible dark>{{ alert_success_text }}</v-alert>
    <v-form v-if="loaded" ref="form">
    <v-toolbar color="primary" dark flat dense>
      <v-tabs v-model="tab">
          <v-spacer/>
          <v-tab value="picture">Picture</v-tab>
          <v-tab value="timing">Timing</v-tab>
          <v-tab value="cellconnection">Cell Connection</v-tab>
          <v-tab value="powersupply">Power Supply</v-tab>
          <v-spacer/>
      </v-tabs>
    </v-toolbar>
    <v-tabs-items v-model="tab">
      <v-tab-item key="picture">
        <v-card class="pt-8" color="grey lighten-3" flat>
          <v-container>
            <v-row>
              <v-col cols="12">
                <div>Quality</div>
                <div class="d-flex flex-row">
                  <v-slider :color="hasChangedCam('cam_quality') ? 'red' : 'primary'" v-model="parameters['cam_quality'].modifiedValue" min="10" max="63"></v-slider>
                  <v-chip @click:close="revert('cam_quality')" :text-color="hasChangedCam('cam_quality') ? 'white' : null" :color="hasChangedCam('cam_quality') ? 'red' : 'primary'" :close="hasChangedCam('cam_quality')" class="ml-2">{{ parameters['cam_quality'].modifiedValue }}</v-chip>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="12">
                <div>Pixel Timing</div>
                <div class="d-flex flex-row">
                  <v-slider :color="hasChangedCam('cam_pixeltiming') ? 'red' : 'primary'" v-model="parameters['cam_pixeltiming'].modifiedValue" min="0" max="14"></v-slider>
                  <v-chip @click:close="revert('cam_pixeltiming')" :text-color="hasChangedCam('cam_pixeltiming') ? 'white' : null" :color="hasChangedCam('cam_pixeltiming') ? 'red' : 'primary'" :close="hasChangedCam('cam_pixeltiming')" class="ml-2">{{ parameters['cam_pixeltiming'].modifiedValue }}</v-chip>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="12">
                <div>Auto Exposure</div>
                <div class="d-flex flex-row">
                  <v-slider :color="hasChangedCam('cam_auto_exposure') ? 'red' : 'primary'" v-model="parameters['cam_auto_exposure'].modifiedValue" min="0" max="4"></v-slider>
                  <v-chip @click:close="revert('cam_auto_exposure')" :text-color="hasChangedCam('cam_auto_exposure') ? 'white' : null" :color="hasChangedCam('cam_auto_exposure') ? 'red' : 'primary'" :close="hasChangedCam('cam_auto_exposure')" class="ml-2">{{ parameters['cam_auto_exposure'].modifiedValue }}</v-chip>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="6">
                <div class="d-flex flex-row">
                  <v-select :color="hasChangedCam('cam_light_mode') ? 'red' : 'primary'" v-model="parameters['cam_light_mode'].modifiedValue" :items="whitebalance_modes"  label="White Balance Mode" />
                  <v-btn @click="revert('cam_light_mode')" v-if="hasChangedCam('cam_light_mode')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="12">
                <div>Brightness</div>
                <div class="d-flex flex-row">
                  <v-slider :color="hasChangedCam('cam_brightness') ? 'red' : 'primary'" v-model="parameters['cam_brightness'].modifiedValue" min="0" max="9"></v-slider>
                  <v-chip @click:close="revert('cam_brightness')" :text-color="hasChangedCam('cam_brightness') ? 'white' : null" :color="hasChangedCam('cam_brightness') ? 'red' : 'primary'" :close="hasChangedCam('cam_brightness')" class="ml-2">{{ parameters['cam_brightness'].modifiedValue }}</v-chip>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="12">
                <div>Contrast</div>
                <div class="d-flex flex-row">
                  <v-slider :color="hasChangedCam('cam_contrast') ? 'red' : 'primary'" v-model="parameters['cam_contrast'].modifiedValue" min="0" max="9"></v-slider>
                  <v-chip @click:close="revert('cam_contrast')" :text-color="hasChangedCam('cam_contrast') ? 'white' : null" :color="hasChangedCam('cam_contrast') ? 'red' : 'primary'" :close="hasChangedCam('cam_contrast')" class="ml-2">{{ parameters['cam_contrast'].modifiedValue }}</v-chip>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="12">
                <div>Hue</div>
                <div class="d-flex flex-row">
                  <v-slider :color="hasChangedCam('cam_hue') ? 'red' : 'primary'" v-model="parameters['cam_hue'].modifiedValue" min="0" max="5"></v-slider>
                  <v-chip @click:close="revert('cam_hue')" :text-color="hasChangedCam('cam_hue') ? 'white' : null" :color="hasChangedCam('cam_hue') ? 'red' : 'primary'" :close="hasChangedCam('cam_hue')" class="ml-2">{{ parameters['cam_hue'].modifiedValue }}</v-chip>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="12">
                <div>Sharpness</div>
                <div class="d-flex flex-row">
                  <v-slider :color="hasChangedCam('cam_sharpness') ? 'red' : 'primary'" v-model="parameters['cam_sharpness'].modifiedValue" min="0" max="7"></v-slider>
                  <v-chip @click:close="revert('cam_sharpness')" :text-color="hasChangedCam('cam_sharpness') ? 'white' : null" :color="hasChangedCam('cam_sharpness') ? 'red' : 'primary'" :close="hasChangedCam('cam_sharpness')" class="ml-2">{{ parameters['cam_sharpness'].modifiedValue }}</v-chip>
                </div>
              </v-col>
            </v-row>
          </v-container>
        </v-card>
      </v-tab-item>

      <v-tab-item key="timing">
        <v-card class="pt-6" color="grey lighten-3" flat>
          <v-radio-group v-model="server_parameters['timing_strategy'].modifiedValue">
          <v-container>
            <v-row>
              <v-col cols="6">
                <div class="d-flex flex-row">
                  <v-text-field :color="hasChangedCam('sys_secs_between_photos') ? 'red' : 'primary'" v-model="parameters['sys_secs_between_photos'].modifiedValue" label="Seconds between Uploads in Regular Mode" />
                  <v-btn @click="revert('sys_secs_between_photos')" v-if="hasChangedCam('sys_secs_between_photos')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="6">
                <div class="d-flex flex-row">
                  <v-text-field v-model="server_parameters['secs_between_photos_override'].modifiedValue" label="Seconds between Uploads Override" />
                  <v-btn @click="revert_server('secs_between_photos_override')" v-if="hasChangedServer('secs_between_photos_override')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                  <v-btn @click="clear_server_parameter('secs_between_photos_override')" class="align-self-center ml-3" color="primary" small>Clear</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="12">
                <v-radio label="Use Fixed Times for First and Last Photo of the Day" value="fixed"></v-radio>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="6">
                <div class="d-flex flex-row ml-6">
                  <v-menu ref="menu_first_photo" v-model="first_photo_menu" :close-on-content-click="false" :nudge-right="40" :return-value.sync="server_parameters['start_time'].modifiedValue" transition="scale-transition" offset-y max-width="290px" min-width="290px">
                    <template v-slot:activator="{ on, attrs }">
                      <v-text-field v-model="server_parameters['start_time'].modifiedValue" label="Time of First Photo" prepend-icon="mdi-clock-time-four-outline" readonly v-bind="attrs" v-on="on"/>
                    </template>
                    <v-time-picker ampm-in-title v-if="first_photo_menu" v-model="server_parameters['start_time'].modifiedValue" :allowed-minutes="timepicker_allowed_minutes" full-width @click:minute="$refs.menu_first_photo.save(server_parameters['start_time'].modifiedValue)"/>
                  </v-menu>
                  <v-btn @click="revert_server('start_time')" v-if="hasChangedServer('start_time')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
              <v-col cols="6">
                <div class="d-flex flex-row">
                  <v-menu ref="menu_last_photo" v-model="last_photo_menu" :close-on-content-click="false" :nudge-right="40" :return-value.sync="server_parameters['end_time'].modifiedValue" transition="scale-transition" offset-y max-width="290px" min-width="290px">
                    <template v-slot:activator="{ on, attrs }">
                      <v-text-field v-model="server_parameters['end_time'].modifiedValue" label="Time of Last Photo" prepend-icon="mdi-clock-time-four-outline" readonly v-bind="attrs" v-on="on"/>
                    </template>
                    <v-time-picker ampm-in-title v-if="last_photo_menu" v-model="server_parameters['end_time'].modifiedValue" :allowed-minutes="timepicker_allowed_minutes" full-width @click:minute="$refs.menu_last_photo.save(server_parameters['end_time'].modifiedValue)"/>
                  </v-menu>
                  <v-btn @click="revert_server('end_time')" v-if="hasChangedServer('end_time')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="12">
                <v-radio label="Use Sunrise / Sunset Times for First and Last Photo of the Day" value="sunlight"></v-radio>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="6">
                <div class="d-flex flex-row ml-6">
                  <v-text-field v-model="server_parameters['latitude'].modifiedValue" label="Latitude" prepend-icon="mdi-map-marker" placeholder="(+/-)## . ######" hint="ex valid entries: -14.356220 +04.788863 00.002377" />
                  <v-btn @click="revert_server('latitude')" v-if="hasChangedServer('latitude')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
              <v-col cols="6">
                <div class="d-flex flex-row">
                  <v-text-field v-model="server_parameters['longitude'].modifiedValue" label="Longitude" prepend-icon="mdi-map-marker" placeholder="(+/-)## . ######" hint="ex valid entries: -14.356220 +04.788863 00.002377" />
                  <v-btn @click="revert_server('longitude')" v-if="hasChangedServer('longitude')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col cols="6">
                <div class="d-flex flex-row ml-6">
                  <v-text-field v-model="server_parameters['offset_firstphoto'].modifiedValue" label="Offset First Photo (secs)" prepend-icon="mdi-map-marker" placeholder="#####"  />
                  <v-btn @click="revert_server('offset_firstphoto')" v-if="hasChangedServer('offset_firstphoto')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
              <v-col cols="6">
                <div class="d-flex flex-row">
                  <v-text-field v-model="server_parameters['offset_lastphoto'].modifiedValue" label="Offset Last Photo (secs)" prepend-icon="mdi-map-marker" placeholder="#####"  />
                  <v-btn @click="revert_server('offset_lastphoto')" v-if="hasChangedServer('offset_lastphoto')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
          </v-container>
        </v-radio-group>
        </v-card>
      </v-tab-item>
      <v-tab-item key="cellconnection">
        <v-card class="pt-6" color="grey lighten-3" flat>
          <v-container>
            <v-row>
              <v-col>
                <div class="d-flex flex-row">
                  <v-text-field :color="hasChangedCam('cell_remote_url') ? 'red' : 'primary'" v-model="parameters['cell_remote_url'].modifiedValue" label="Upload URL" />
                  <v-btn @click="revert('cell_remote_url')" v-if="hasChangedCam('cell_remote_url')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col>
                <div class="d-flex flex-row">
                  <v-text-field :color="hasChangedCam('cell_pin') ? 'red' : 'primary'" v-model="parameters['cell_pin'].modifiedValue" :rules="pinRules" label="SIM PIN (If SIM card has no pin set, use any value as this setting will be ignored.)" />
                  <v-btn @click="revert('cell_pin')" v-if="hasChangedCam('cell_pin')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col>
                <div class="d-flex flex-row">
                  <v-text-field :color="hasChangedCam('cell_apn') ? 'red' : 'primary'" v-model="parameters['cell_apn'].modifiedValue" label="APN" />
                  <v-btn @click="revert('cell_apn')" v-if="hasChangedCam('cell_apn')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col>
                <div class="d-flex flex-row">
                  <v-text-field :color="hasChangedCam('cell_apn_user') ? 'red' : 'primary'" v-model="parameters['cell_apn_user'].modifiedValue" label="APN User" />
                  <v-btn @click="revert('cell_apn_user')" v-if="hasChangedCam('cell_apn_user')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col>
                <div class="d-flex flex-row">
                  <v-text-field :color="hasChangedCam('cell_apn_pass') ? 'red' : 'primary'" v-model="parameters['cell_apn_pass'].modifiedValue" label="APN Password" />
                  <v-btn @click="revert('cell_apn_pass')" v-if="hasChangedCam('cell_apn_pass')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
            <v-row>
              <v-col>
                <div class="d-flex flex-row">
                  <v-select :color="hasChangedCam('cell_apn_auth') ? 'red' : 'primary'" v-model="parameters['cell_apn_auth'].modifiedValue" :items="apnauth_modes"  label="APN Auth Type" />
                  <v-btn @click="revert('cell_apn_auth')" v-if="hasChangedCam('cell_apn_auth')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
          </v-container>
        </v-card>
      </v-tab-item>

      <v-tab-item key="powersupply">
        <v-card class="pt-6" color="grey lighten-3" flat>
          <v-container>
            <v-row>
              <v-col cols="6">
                <div class="d-flex flex-row">
                  <v-text-field :color="hasChangedCam('sys_minimum_voltage') ? 'red' : 'primary'" v-model="parameters['sys_minimum_voltage'].modifiedValue" label="Minimum Battery Voltage (mV)" />
                  <v-btn @click="revert('sys_minimum_voltage')" v-if="hasChangedCam('sys_minimum_voltage')" class="align-self-center ml-3" small color="red" dark>Revert</v-btn>
                </div>
              </v-col>
            </v-row>
          </v-container>
        </v-card>
      </v-tab-item>
    </v-tabs-items>
    <v-card class="float-right my-4" flat>
      <v-btn dark color="primary" @click="apply">Save and Upload during next Cycle</v-btn>
    </v-card>
  </v-form>
  </v-container>
</template>

<script>
import axios from 'axios';
import store from "../store";
import { AUTH_LOGOUT } from "../store/actions/auth";

export default {
  name: 'SettingsForm',
  components: {
  },
  data: () => ({
    alert_success: false,
    alert_success_text: null,
    alert_error: false,
    alert_error_text: null,
    loaded: false,
    pinRules: [
        value => (parseInt(value) >= 0 | value == '') || 'Must be an integer with at least four digits or empty',
      ],
    tab: null,
    whitebalance_modes: [
      { text: 'Auto', value: 0 },
      { text: 'Sunny', value: 1 },
      { text: 'Cloudy', value: 2 },
      { text: 'Office', value: 3 },
      { text: 'Home', value: 4 }
    ],
    apnauth_modes: [
      { text: 'None', value: 0 },
      { text: 'PAP', value: 1 },
      { text: 'CHAP', value: 2 }
    ],
    timepicker_allowed_minutes: [0, 15, 30, 45],
    first_photo_menu: false,
    last_photo_menu: false,
    server_parameters: {
      "start_time": { activeValue: null, modifiedValue: null, defaultValue: "06:00" },
      "end_time": { activeValue: null, modifiedValue: null, defaultValue: "22:00" },
      "timing_strategy": { activeValue: null, modifiedValue: null, defaultValue: "fixed" },
      "latitude": { activeValue: null, modifiedValue: null, defaultValue: 0.0 },
      "longitude": { activeValue: null, modifiedValue: null, defaultValue: 0.0 },
      "offset_firstphoto": { activeValue: null, modifiedValue: null, defaultValue: 0 },
      "offset_lastphoto": { activeValue: null, modifiedValue: null, defaultValue: 0 },
      "secs_between_photos_override": { activeValue: null, modifiedValue: null, defaultValue: null },
    },
    parameters: {
      "cam_quality": { activeValue: 10, modifiedValue: 10, modifiedValueServer: 10, defaultValue: 10 },
      "cam_pixeltiming": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 0 },
      "cam_auto_exposure": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 0 },
      "cam_light_mode": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 0 },
      "cam_color_saturation": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 0 },
      "cam_brightness": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 0 },
      "cam_contrast": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 0 },
      "cam_hue": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 0 },
      "cam_sharpness": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 0 },
      "cell_pin": { activeValue: "", modifiedValue: "", modifiedValueServer: "", defaultValue: "1000" },
      "cell_apn": { activeValue: "", modifiedValue: "", modifiedValueServer: "", defaultValue: "internet" },
      "cell_apn_user": { activeValue: "", modifiedValue: "", modifiedValueServer: "", defaultValue: "" },
      "cell_apn_pass": { activeValue: "", modifiedValue: "", modifiedValueServer: "", defaultValue: "" },
      "cell_apn_auth": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 0 },
      "cell_remote_url": { activeValue: "", modifiedValue: "", modifiedValueServer: "", defaultValue: "http://wwv-schwarzwald.de/webcam/api/upload" },
      "sys_secs_between_photos": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 3600 },
      "sys_minimum_voltage": { activeValue: 0, modifiedValue: 0, modifiedValueServer: 0, defaultValue: 10400 }
    },
  }),
  methods: {
    whitebalance_mode_text_from_value(value) {
      const found=this.whitebalance_modes.find(element => element.value == value)
      return found.text
    },
    hasChangedServer(parameter) {
      if (this.server_parameters[parameter]) {
        return this.server_parameters[parameter].activeValue != this.server_parameters[parameter].modifiedValue
      } else return false
    },
    hasChangedCam(parameter) {
      if (this.parameters[parameter]) {
        return this.parameters[parameter].activeValue != this.parameters[parameter].modifiedValue
      } else return false
    },
    needsToBeSaved(parameter) {
      if (this.parameters[parameter]) {
        var returnValue = false;
        if (this.parameters[parameter].activeValue != this.parameters[parameter].modifiedValue) returnValue = true
        if (this.parameters[parameter].modifiedValueServer != this.parameters[parameter].modifiedValue) returnValue = true
        return returnValue
      }
      return false
    },
    revert(parameter) {
      this.parameters[parameter].modifiedValue = this.parameters[parameter].activeValue;
    },
    revert_server(parameter) {
      this.server_parameters[parameter].modifiedValue = this.server_parameters[parameter].activeValue;
    },
    clear_server_parameter(parameter) {
      this.server_parameters[parameter].modifiedValue = this.server_parameters[parameter].defaultValue
    },
    apply() {
      // create array for cam config
      var newCamConfig = {}
      for (var camKey in this.parameters) {
        if (this.needsToBeSaved(camKey)) {
          newCamConfig[camKey] = this.parameters[camKey].modifiedValue
        }
      }
      // create array for server config
      var newServerConfig = {}
      for (var serverKey in this.server_parameters) {
        newServerConfig[serverKey] = this.server_parameters[serverKey].modifiedValue
      }

      // post to backend
      axios.post(process.env['VUE_APP_BACKENDURL']+'/set_config', {
        token: store.getters.currentToken,
        server: newServerConfig,
        cam: newCamConfig }
      )
      .then((response) => {
        this.loadFromServer(response.data)
        this.alert_success_text = "Changes successfully saved and scheduled for transmission."
        this.alert_success = true
        this.alert_error = false
        window.scrollTo(0,0);
      }, (error) => {
        if (error.response.status == 401) {
          // session expired
          this.$store.dispatch(AUTH_LOGOUT).then(() => {
            this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
          }).catch(() => { /* TODO */ });
        }
        this.loaded = false
        this.alert_error_text = error.message
        this.alert_error = true
        this.alert_success = false
        window.scrollTo(0,0);
      });
    },

    loadFromServer (data) {
      // server parameters
      for (var server_key in this.server_parameters) {
        if (data.server && data.server[server_key]) {
          this.server_parameters[server_key].activeValue = data.server[server_key];
          this.server_parameters[server_key].modifiedValue = data.server[server_key];
        }
      }
      // cam parameters
      for (var key in this.parameters) {
        if (data.active && data.active[key]) {
          this.parameters[key].activeValue = data.active[key]
        } else {
          this.parameters[key].activeValue = this.parameters[key].defaultValue
        }
        if (data.modified && data.modified[key]) {
          this.parameters[key].modifiedValue = data.modified[key]
          this.parameters[key].modifiedValueServer = data.modified[key]
        } else {
          this.parameters[key].modifiedValue = this.parameters[key].activeValue
          this.parameters[key].modifiedValueServer = this.parameters[key].activeValue
        }
      }
    }
  },
  mounted () {
    axios.post(process.env['VUE_APP_BACKENDURL']+'/get_config', { token: store.getters.currentToken }
    )
    .then((response) => {
      this.loadFromServer(response.data)
      this.loaded = true
      this.alert_success = false
      this.alert_error = false
    }, (error) => {
      if (error.response.status == 401) {
        // session expired
        this.$store.dispatch(AUTH_LOGOUT).then(() => {
          this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
        }).catch(() => { /* TODO */ });
      }
      this.loaded = false
      this.alert_error_text = error.message
      this.alert_error = true
      this.alert_success = false
    })
  }
};
</script>


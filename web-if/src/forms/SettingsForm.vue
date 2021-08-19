<template>
  <v-container>
    <h2 class="py-4">Modify Camera Settings</h2>
    <v-alert v-if="error != null" border="top" color="red lighten-2" dark>{{ error }}</v-alert>
    <v-form v-if="loaded" ref="form">
    <v-toolbar color="indigo" dark flat dense>
      <v-tabs v-model="tab">
          <v-spacer/>
          <v-tab value="picture">Picture</v-tab>
          <v-tab value="upload">Upload</v-tab>
          <v-spacer/>
      </v-tabs>
    </v-toolbar>
    <v-tabs-items v-model="tab">
      <v-tab-item key="picture">
        <v-card class="pt-6" color="grey lighten-3" flat>
          <v-container>
            <v-row>
              <v-col>
                <v-slider :color="hasChanged('cam_quality') ? 'red' : 'indigo'" v-model="parameters['cam_quality'].modifiedValue" thumb-label min="10" max="63" label="Quality"></v-slider>
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-slider :color="hasChanged('cam_auto_exposure') ? 'red' : 'indigo'" v-model="parameters['cam_auto_exposure'].modifiedValue" thumb-label min="0" max="4" label="Auto Exposure" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-select :color="hasChanged('cam_light_mode') ? 'red' : 'indigo'" v-model="parameters['cam_light_mode'].modifiedValue" :items="whitebalance_modes"  label="White Balance Mode" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-slider :color="hasChanged('cam_brightness') ? 'red' : 'indigo'" v-model="parameters['cam_brightness'].modifiedValue" thumb-label min="0" max="9" label="Brightness" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-slider :color="hasChanged('cam_contrast') ? 'red' : 'indigo'" v-model="parameters['cam_contrast'].modifiedValue" thumb-label min="0" max="9" label="Contrast" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-slider :color="hasChanged('cam_hue') ? 'red' : 'indigo'" v-model="parameters['cam_hue'].modifiedValue" thumb-label min="0" max="5" label="Hue" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-slider :color="hasChanged('cam_sharpness') ? 'red' : 'indigo'" v-model="parameters['cam_sharpness'].modifiedValue" thumb-label min="0" max="7" label="Sharpness" />
              </v-col>
            </v-row>
          </v-container>
        </v-card>
      </v-tab-item>
      <v-tab-item key="upload">
        <v-card class="pt-6" color="grey lighten-3" flat>
          <v-container>
            <v-row>
              <v-col>
                <v-text-field :color="hasChanged('cell_remote_url') ? 'red' : 'indigo'" v-model="parameters['cell_remote_url'].modifiedValue" label="Upload URL" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-text-field :color="hasChanged('cell_pin') ? 'red' : 'indigo'" v-model="parameters['cell_pin'].modifiedValue" :rules="pinRules" label="SIM PIN" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-text-field :color="hasChanged('cell_apn') ? 'red' : 'indigo'" v-model="parameters['cell_apn'].modifiedValue" label="APN" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-text-field :color="hasChanged('cell_apn_user') ? 'red' : 'indigo'" v-model="parameters['cell_apn_user'].modifiedValue" label="APN User" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-text-field :color="hasChanged('cell_apn_pass') ? 'red' : 'indigo'" v-model="parameters['cell_apn_pass'].modifiedValue" label="APN Password" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-select :color="hasChanged('cell_apn_auth') ? 'red' : 'indigo'" v-model="parameters['cell_apn_auth'].modifiedValue" :items="apnauth_modes"  label="APN Auth Type" />
              </v-col>
            </v-row>
          </v-container>
          <v-container>
            <v-row>
              <v-col>
                <v-text-field :color="hasChanged('sys_secs_between_photos') ? 'red' : 'indigo'" v-model="parameters['sys_secs_between_photos'].modifiedValue" label="Seconds between Uploads" />
              </v-col>
            </v-row>
          </v-container>
        </v-card>
      </v-tab-item>
    </v-tabs-items>
    <v-card class="float-right my-4" flat>
      <v-btn dark color="indigo" @click="apply">Save and Upload during next Cycle</v-btn>
      <v-btn dark color="indigo" @click="discard" class="ml-5">Discard</v-btn>
    </v-card>
  </v-form>
  </v-container>
</template>

<script>
import axios from 'axios';
import store from "../store";

export default {
  name: 'SettingsForm',
  components: {
  },
  data: () => ({
    error: null,
    loaded: false,
    pinRules: [
        value => (parseInt(value) >= 0 | value == '') || 'Must be an integer with at least five digits or empty',
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
    parameters: {
      "cam_quality": { activeValue: 0, modifiedValue: 0, defaultValue: 10 },
      "cam_auto_exposure": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_light_mode": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_color_saturation": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_brightness": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_contrast": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_hue": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cam_sharpness": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cell_pin": { activeValue: "", modifiedValue: "", defaultValue: "1000" },
      "cell_apn": { activeValue: "", modifiedValue: "", defaultValue: "internet" },
      "cell_apn_user": { activeValue: "", modifiedValue: "", defaultValue: "" },
      "cell_apn_pass": { activeValue: "", modifiedValue: "", defaultValue: "" },
      "cell_apn_auth": { activeValue: 0, modifiedValue: 0, defaultValue: 0 },
      "cell_remote_url": { activeValue: "", modifiedValue: "", defaultValue: "http://wwv-schwarzwald.de/webcam/api/upload" },
      "sys_secs_between_photos": { activeValue: 0, modifiedValue: 0, defaultValue: 3600 },
    },

  }),
  methods: {
    hasChanged(parameter) {
      if (this.parameters[parameter]) {
        return this.parameters[parameter].activeValue != this.parameters[parameter].modifiedValue
      } else return false
    },
    apply() {
      // create array
      var newConfig = {}
      for (var key in this.parameters) {
        if (this.hasChanged(key)) {
          newConfig[key] = this.parameters[key].modifiedValue
        }
      }
      console.log(JSON.stringify(newConfig))
      // post to backend
      axios.post(process.env['VUE_APP_BACKENDURL']+'/set_config', JSON.stringify(newConfig), {
        auth: {
            username: store.getters.currentUser,
            password: store.getters.currentPassword
        }})
      .then((response) => {
        console.log(response);
      }, (error) => {
        console.log(error);
      });
      this.$router.push('/photos')
    },
    discard() {
      this.$router.push('/photos')
    }
  },
  mounted () {
    axios.get(process.env['VUE_APP_BACKENDURL']+'/get_config', {
      auth: {
          username: store.getters.currentUser,
          password: store.getters.currentPassword
      }})
    .then((response) => {
      for (var key in this.parameters) {
        if (response.data.active && response.data.active[key]) {
          this.parameters[key].activeValue = response.data.active[key]
        } else {
          this.parameters[key].activeValue = this.parameters[key].defaultValue
        }
        if (response.data.modified && response.data.modified[key]) {
          this.parameters[key].modifiedValue = response.data.modified[key]
        } else {
          this.parameters[key].modifiedValue = this.parameters[key].activeValue
        }
      }
      this.loaded = true
    }, (error) => {
      this.error = "Unable to load data: " + error.message
    });
}
};
</script>


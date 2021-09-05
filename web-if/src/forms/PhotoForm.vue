<template>
  <v-container class="my-3 mx-auto">
    <v-alert v-model="alert_error" border="top" color="red lighten-2" class="mb-5" dark>{{ alert_error_text }}</v-alert>
    <v-alert v-model="alert_success" border="top" color="green lighten-2" class="mb-5" dark>{{ alert_success_text }}</v-alert>
    <v-alert v-model="alert_new_photos" border="top" color="green lighten-2" class="mb-5" dark>
    <v-row align="center">
      <v-col class="grow">UPDATE: New photos are available</v-col>
      <v-col class="shrink"><v-btn @click="gotoLatestPhoto" dark color="green darken-2">Go to latest photo</v-btn></v-col>
    </v-row>
    </v-alert>
    <v-row class="my-3">
      <v-toolbar dense flat>
        <v-btn small :disabled="photoidx<=0" @click="prevPhoto" dark color="primary"><v-icon>mdi-chevron-left</v-icon><span class="d-none d-md-block">Prev. Photo</span></v-btn>
        <v-spacer/>
        <DayPicker ref="dayPicker" v-model="day" />
        <v-select v-model="photo" :items="photos" label="Time">
            <template slot="selection" slot-scope="data">
            {{ new Date(data.item.timestamp * 1000).toLocaleTimeString("en-US",  ) }}
            </template>
            <template slot="item" slot-scope="data">
            {{ new Date(data.item.timestamp * 1000).toLocaleTimeString("en-US",  ) }}
            </template>
        </v-select>
        <v-spacer/>
        <v-btn small :disabled="photoidx>=(photos.length-1)" @click="nextPhoto" dark color="primary"><span class="d-none d-md-block">Next Photo</span><v-icon>mdi-chevron-right</v-icon></v-btn>
      </v-toolbar>
    </v-row>
    <v-row>
      <Photo v-model="photo" @deleted="deletePhoto" />
    </v-row>
  </v-container>
</template>

<script>
import DayPicker from '../components/DayPicker.vue';
import Photo from '../components/Photo'
import axios from 'axios';
import store from "../store";
import { AUTH_LOGOUT } from "../store/actions/auth";

export default {
  name: 'PhotoForm',
  components: {
    Photo,
    DayPicker
  },
  data: () => ({
    alert_new_photos: false,
    alert_error: false,
    alert_success: false,
    alert_error_text: null,
    alert_success_text: null,
    day: null,
    photos: [],
    photo: null,
    photoidx: null,
    authenticated: false,
    updateInterval: null
  }),
  methods: {
    gotoLatestPhoto: function() {
      const currentDate = new Date()
      const current_day  = this.day
      this.day = currentDate.getFullYear().toString() + "-" +
        new String((currentDate.getMonth()+1).toString()).padStart(2, "0") + "-" +
        new String(currentDate.getDate().toString()).padStart(2, "0)")
      this.alert_error = false
      this.alert_success = false
      this.alert_new_photos = false
      if (current_day == this.day) this.loadDay(this.day)
    },
    deletePhoto: function(id) {
      axios.post(process.env['VUE_APP_BACKENDURL']+'/photo_delete', { id: id, token: store.getters.currentToken },
      ).then(() => {
        // reload day to switch to next available photo
        this.loadDay(this.day)
        this.alert_error = false
        this.alert_success = true
        this.alert_success_text = "Photo successfully deleted"
      }, (error) => {
        if (error.response.status == 401) {
          // session expired
          this.$store.dispatch(AUTH_LOGOUT).then(() => {
            this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
          }).catch(() => { /* TODO */ });
        } else {
          this.alert_error = true
          this.alert_success = false
          this.alert_error_text = "Error deleting photo: " + error.message
          // delete failed
        }
      })
    },
    nextPhoto: function() {
      if (this.photoidx < (this.photos.length - 1)) {
        this.photoidx++
        this.photo = this.photos[this.photoidx]
      }
    },
    prevPhoto: function() {
      if (this.photoidx > 0) {
        this.photoidx--
        this.photo = this.photos[this.photoidx]
      }
    },
    latestPhoto: function() {
      this.$refs.dayPicker.gotoLatest()
    },
    loadDay: function(date) {
      // construct date
      axios.post(process.env['VUE_APP_BACKENDURL']+'/photos_per_day', { date: date, token: store.getters.currentToken },
      ).then(response => {
        this.photos = response.data
        this.photoidx = this.photos.length - 1
        this.photo = this.photos[this.photoidx]
      }, (error) => {
        if (error.response.status == 401) {
          // session expired
          this.$store.dispatch(AUTH_LOGOUT).then(() => {
            this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
          }).catch(() => { /* TODO */ });
        }
      });
    },
  },
  watch: {
    photo: function(val) {
      this.photoidx = this.photos.findIndex(data => data === val)
    },
    day: function(val) {
      if (val) {
        this.loadDay(val)
      } else {
        this.photos = []
        this.photoidx = null
        this.photo = null
      }
    }
  },
  created() {
    this.updateInterval = setInterval(function() {
      axios.post(process.env['VUE_APP_BACKENDURL']+'/photo_newavailable', { duration: 35, token: store.getters.currentToken },
      ).then((response) => {
        if (response.data && response.data.available && response.data.available > 0) {
          // new photos are available
          this.alert_new_photos = true
          this.alert_error = false
          this.alert_success = false
        }
      }, (error) => {
        if (error.response.status == 401) {
          // session expired
          this.$store.dispatch(AUTH_LOGOUT).then(() => {
            this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
          }).catch(() => { /* TODO */ });
        } else {
          this.alert_error = true
          this.alert_success = false
          this.alert_error_text = "Error looking for new photos: " + error.message
        }})
    }.bind(this), 30000)

    const currentDate = new Date()
    this.day = currentDate.getFullYear().toString() + "-" +
      new String((currentDate.getMonth()+1).toString()).padStart(2, "0") + "-" +
      new String(currentDate.getDate().toString()).padStart(2, "0)")

  },
  destroyed() {
    clearInterval(this.updateInterval)
  },
  mounted () {
  }
};
</script>


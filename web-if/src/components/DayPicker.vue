<template>
    <v-menu
        v-model="menu"
        :close-on-content-click="false"
        :nudge-right="40"
        transition="scale-transition"
        offset-y
        min-width="auto"
      >
        <template v-slot:activator="{ on, attrs }">
          <v-text-field
            v-model="date"
            label="Day"
            readonly
            v-bind="attrs"
            v-on="on"
          ></v-text-field>
        </template>
        <v-date-picker v-model="date" @input="menu=false" :picker-date.sync="pickerDate" event-color="green lighten-1" :events="daysWithPhotos" />
    </v-menu>
</template>
<script>
  import axios from 'axios'
  import store from "../store"
  import { AUTH_LOGOUT } from "../store/actions/auth";

  export default {
    name: "DayPicker",
    props: [ 'value' ],
    methods: {
      updateDate: function(val) {
        this.$emit('input', val)
      },
      loadDaysWithPictures: function(year, month) {
        axios.post(process.env['VUE_APP_BACKENDURL']+'/photos_per_month',
        { year: year, month: month, token: store.getters.currentToken}
      ).then(response => {
          this.daysWithPhotos = []
          response.data.forEach(element => {
            this.daysWithPhotos.push(element.substr(0,10))
          })
        }, (error) => {
            if (error.response.status == 401) {
              // session expired
              this.$store.dispatch(AUTH_LOGOUT).then(() => {
                this.$router.push({name: 'Login', params: { message: 'Your session expired.'}})
              }).catch(() => { /* TODO */ });
            }
        });
      }
    },
    watch: {
      pickerDate: function () {
        this.validDates = []
        let selectedMonth = new Date(this.pickerDate).getMonth() + 1
        let selectedYear = new Date(this.pickerDate).getFullYear()
        this.loadDaysWithPictures(selectedYear, selectedMonth)
      },
      date: function(val) {
        if (val) {
          this.updateDate(val)
        } else {
          this.updateDate(null)
        }
      },
    },
    data: () => ({
      pickerDate: null,
      menu: false,
      daysWithPhotos: [],
      date: null
    }),
    mounted () {
      this.date = this.value
    },
  }
</script>